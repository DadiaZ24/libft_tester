#include "tester.h"
#include <signal.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

/*
** ======================================================================
**  MALLOC TRACKER
** ======================================================================
** malloc() and free() are redefined here. libft.a is linked statically
** into this executable, so every malloc/free inside your libft lands in
** these functions.
**
** While a test runs (always inside a forked child) every block gets:
**   - its bytes filled with 0xBE     -> forgetting a '\0' or a ->next = NULL
**                                        shows up instead of "working by luck"
**   - 16 canary bytes after the end  -> writing 1 byte too far is caught
**   - an entry in a table            -> leaks, double frees, "is this really
**                                        a malloc'd pointer?" can be checked
** free() never gives the memory back: the block is filled with 0xDF and
** kept (quarantine). Reading a freed node's ->next gives 0xDFDFDFDF... and
** crashes right away; writing into freed memory is found at the end.
*/

#ifdef __GLIBC__
void	*__libc_malloc(size_t n);
void	__libc_free(void *p);
# define REAL_MALLOC(n) __libc_malloc(n)
# define REAL_FREE(p) __libc_free(p)
#else
# include <dlfcn.h>
static void	*real_malloc(size_t n)
{
	static void	*(*f)(size_t);

	if (!f)
		f = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
	return (f(n));
}
static void	real_free(void *p)
{
	static void	(*f)(void *);

	if (!f)
		f = (void (*)(void *))dlsym(RTLD_NEXT, "free");
	f(p);
}
# define REAL_MALLOC(n) real_malloc(n)
# define REAL_FREE(p) real_free(p)
#endif

#define CANARY		16
#define FILL_NEW	0xBE
#define FILL_CANARY	0xCA
#define FILL_FREED	0xDF
#define MAX_ALLOC	((size_t)1 << 30)
#define TAB_BITS	23
#define TAB_SIZE	((size_t)1 << TAB_BITS)
#define MAX_BLOCKS	(TAB_SIZE / 2)

#define ST_LIVE		1
#define ST_FREED	2

typedef struct s_blk
{
	unsigned char	*p;
	uint32_t		size;
	uint32_t		state;
}	t_blk;

static t_blk		*g_tab;
static uint32_t		*g_order;
static size_t		g_nblocks;
static int			g_track;
static int			g_armed;
static long			g_fail_at = -1;
static long			g_calls;
static int			g_injected;

/* child <-> parent reporting */
static int			g_out = -1;
static int			g_nfails;
static char			g_step[512];

static void	emit(char type, const char *msg);
static void	emit_crash(const char *what);

static size_t	hash_ptr(const void *p)
{
	uint64_t	h;

	h = (uint64_t)(uintptr_t)p >> 4;
	h *= 0x9E3779B97F4A7C15ULL;
	return ((size_t)(h >> (64 - TAB_BITS)));
}

static void	track_init(void)
{
	g_tab = mmap(NULL, TAB_SIZE * sizeof(t_blk), PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
	g_order = mmap(NULL, MAX_BLOCKS * sizeof(uint32_t), PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
	if (g_tab == MAP_FAILED || g_order == MAP_FAILED)
	{
		g_tab = NULL;
		g_order = NULL;
	}
}

static t_blk	*tab_find(const void *p)
{
	size_t	i;

	if (!g_tab)
		return (NULL);
	i = hash_ptr(p);
	while (g_tab[i].p)
	{
		if (g_tab[i].p == p)
			return (&g_tab[i]);
		i = (i + 1) & (TAB_SIZE - 1);
	}
	return (NULL);
}

static int	tab_insert(unsigned char *p, size_t size)
{
	size_t	i;

	if (!g_tab || g_nblocks >= MAX_BLOCKS)
		return (0);
	i = hash_ptr(p);
	while (g_tab[i].p)
		i = (i + 1) & (TAB_SIZE - 1);
	g_tab[i].p = p;
	g_tab[i].size = (uint32_t)size;
	g_tab[i].state = ST_LIVE;
	g_order[g_nblocks++] = (uint32_t)i;
	return (1);
}

static void	fill(unsigned char *p, int c, size_t n)
{
	while (n--)
		*p++ = (unsigned char)c;
}

static int	canary_ok(const t_blk *b)
{
	size_t	i;

	i = 0;
	while (i < CANARY)
		if (b->p[b->size + i++] != FILL_CANARY)
			return (0);
	return (1);
}

void	*malloc(size_t size)
{
	unsigned char	*p;

	if (!g_track)
		return (REAL_MALLOC(size));
	if (g_armed)
	{
		if (g_fail_at >= 0 && g_calls == g_fail_at)
		{
			g_calls++;
			g_injected = 1;
			return (NULL);
		}
		g_calls++;
	}
	if (size > MAX_ALLOC)
		return (NULL);
	p = REAL_MALLOC(size + CANARY);
	if (!p)
		return (NULL);
	fill(p, FILL_NEW, size);
	fill(p + size, FILL_CANARY, CANARY);
	tab_insert(p, size);
	return (p);
}

void	free(void *ptr)
{
	t_blk	*b;
	char	msg[160];

	if (!ptr)
		return ;
	b = g_track ? tab_find(ptr) : NULL;
	if (!b)
	{
		REAL_FREE(ptr);
		return ;
	}
	if (b->state == ST_FREED)
	{
		snprintf(msg, sizeof(msg), "DOUBLE FREE of a %u-byte block", b->size);
		emit_crash(msg);
		_exit(99);
	}
	if (!canary_ok(b))
	{
		g_nfails++;
		snprintf(msg, sizeof(msg), "HEAP BUFFER OVERFLOW: something wrote past the end of a %u-byte block (seen at free)", b->size);
		emit('H', msg);
	}
	fill(b->p, FILL_FREED, b->size);
	b->state = ST_FREED;
}

int	t_is_block(const void *p)
{
	t_blk	*b;

	b = p ? tab_find(p) : NULL;
	return (b && b->state == ST_LIVE);
}

size_t	t_block_size(const void *p)
{
	t_blk	*b;

	b = p ? tab_find(p) : NULL;
	return (b ? b->size : 0);
}

int	t_is_freed(const void *p)
{
	t_blk	*b;

	b = p ? tab_find(p) : NULL;
	return (b && b->state == ST_FREED);
}

void	t_free(void *p)
{
	if (t_is_block(p))
		free(p);
}

void	t_free_split(char **arr)
{
	size_t	i;

	if (!t_is_block(arr))
		return ;
	i = 0;
	while (i < t_block_size(arr) / sizeof(char *) && arr[i])
		t_free(arr[i++]);
	free(arr);
}

long	t_malloc_calls(void)
{
	return (g_calls);
}

void	t_arm(void)
{
	g_armed = 1;
}

void	t_disarm(void)
{
	g_armed = 0;
}

int	t_injected(void)
{
	return (g_injected);
}

int	t_injecting(void)
{
	return (g_fail_at >= 0);
}

/* Leaks, overflows of blocks never freed, and writes into freed blocks. */
static void	final_heap_check(void)
{
	size_t	i;
	size_t	j;
	size_t	leaks;
	size_t	bytes;
	int		overflow;
	int		uaf;
	char	sizes[200];
	char	msg[400];
	size_t	len;
	t_blk	*b;

	leaks = 0;
	bytes = 0;
	overflow = 0;
	uaf = 0;
	sizes[0] = 0;
	len = 0;
	i = 0;
	while (i < g_nblocks)
	{
		b = &g_tab[g_order[i++]];
		if (!canary_ok(b) && b->state == ST_LIVE)
			overflow++;
		if (b->state == ST_LIVE)
		{
			leaks++;
			bytes += b->size;
			if (leaks <= 8 && len < sizeof(sizes) - 20)
				len += snprintf(sizes + len, sizeof(sizes) - len, "%s%u", leaks > 1 ? ", " : "", b->size);
		}
		else
		{
			j = 0;
			while (j < b->size && b->p[j] == FILL_FREED)
				j++;
			if (j < b->size)
				uaf++;
		}
	}
	if (overflow)
	{
		g_nfails++;
		snprintf(msg, sizeof(msg), "HEAP BUFFER OVERFLOW: %d block(s) were written past their end", overflow);
		emit('H', msg);
	}
	if (uaf)
	{
		g_nfails++;
		snprintf(msg, sizeof(msg), "WRITE AFTER FREE: %d freed block(s) were modified after free()", uaf);
		emit('H', msg);
	}
	if (leaks)
	{
		g_nfails++;
		snprintf(msg, sizeof(msg), "MEMORY LEAK: %zu block(s) / %zu byte(s) never freed (sizes: %s%s)",
			leaks, bytes, sizes, leaks > 8 ? ", ..." : "");
		emit('L', msg);
	}
}

/*
** ======================================================================
**  CHILD SIDE: reporting
** ======================================================================
*/

static void	emit(char type, const char *msg)
{
	char	buf[1200];
	size_t	n;

	if (g_out < 0)
		return ;
	buf[0] = type;
	buf[1] = '|';
	n = 2;
	if (g_step[0] && type != 'N' && type != 'I' && type != 'L')
	{
		n += snprintf(buf + n, sizeof(buf) - n - 2, "%s  ->  ", g_step);
		if (n > sizeof(buf) - 2)
			n = sizeof(buf) - 2;
	}
	n += snprintf(buf + n, sizeof(buf) - n - 1, "%s", msg);
	if (n > sizeof(buf) - 2)
		n = sizeof(buf) - 2;
	buf[n++] = '\n';
	(void)!write(g_out, buf, n);
}

/* async-signal-safe version, used from the signal handler */
static void	emit_crash(const char *what)
{
	char	buf[800];
	size_t	n;
	size_t	i;

	n = 0;
	buf[n++] = 'C';
	buf[n++] = '|';
	i = 0;
	while (what[i] && n < 300)
		buf[n++] = what[i++];
	if (g_step[0])
	{
		i = 0;
		while (" while running: "[i])
			buf[n++] = " while running: "[i++];
		i = 0;
		while (g_step[i] && n < sizeof(buf) - 2)
			buf[n++] = g_step[i++];
	}
	buf[n++] = '\n';
	(void)!write(g_out, buf, n);
}

void	t_case(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vsnprintf(g_step, sizeof(g_step), fmt, ap);
	va_end(ap);
}

void	t_fail(const char *fmt, ...)
{
	va_list	ap;
	char	msg[800];

	g_nfails++;
	if (g_nfails > 8)
		return ;
	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);
	emit('F', msg);
}

const char	*t_escn(const char *s, size_t n)
{
	static char		bufs[6][256];
	static int		k;
	char			*out;
	size_t			i;
	size_t			o;
	unsigned char	c;

	out = bufs[k++ % 6];
	if (!s)
		return ("(NULL)");
	i = 0;
	o = 0;
	while (i < n && o < 200)
	{
		c = (unsigned char)s[i++];
		if (c == '\n')
			o += snprintf(out + o, 256 - o, "\\n");
		else if (c == '\t')
			o += snprintf(out + o, 256 - o, "\\t");
		else if (c == '"' || c == '\\')
			o += snprintf(out + o, 256 - o, "\\%c", c);
		else if (c < 32 || c >= 127)
			o += snprintf(out + o, 256 - o, "\\x%02x", c);
		else
			out[o++] = c;
	}
	if (i < n)
		o += snprintf(out + o, 256 - o, "...");
	out[o] = 0;
	return (out);
}

const char	*t_esc(const char *s)
{
	size_t	n;

	if (!s)
		return ("(NULL)");
	n = 0;
	while (n < 200 && s[n])
		n++;
	return (t_escn(s, n));
}

void	t_check_str(const char *what, char *got, const char *exp)
{
	size_t	need;

	(void)what;
	if (!exp)
	{
		EXPECT(got == NULL, "expected NULL, got \"%s\"", t_esc(got));
		t_free(got);
		return ;
	}
	if (!got)
	{
		t_fail("returned NULL, expected \"%s\"", t_esc(exp));
		return ;
	}
	need = strlen(exp) + 1;
	if (t_is_freed(got))
	{
		t_fail("returned a pointer to memory that was already FREED");
		return ;
	}
	if (!t_is_block(got))
	{
		t_fail("result is not the start of a malloc'd block (string literal? pointer into the input?) -> free() on it would crash");
		return ;
	}
	if (t_block_size(got) < need)
	{
		t_fail("allocated %zu byte(s) but \"%s\" needs %zu (don't forget the '\\0')",
			t_block_size(got), t_esc(exp), need);
		free(got);
		return ;
	}
	if (memcmp(got, exp, need) != 0)
		t_fail("expected \"%s\", got \"%s\"", t_esc(exp), t_escn(got, need - 1));
	free(got);
}

/*
** ======================================================================
**  GUARD PAGES
** ======================================================================
*/

static void	*guard_alloc(size_t n, int front)
{
	size_t			pg;
	size_t			pages;
	unsigned char	*base;

	pg = (size_t)sysconf(_SC_PAGESIZE);
	pages = (n + pg - 1) / pg;
	if (front && pages == 0)
		pages = 1;
	base = mmap(NULL, (pages + 1) * pg, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (base == MAP_FAILED)
	{
		emit_crash("tester: mmap failed (too many guard pages?)");
		_exit(98);
	}
	if (front)
	{
		mprotect(base, pg, PROT_NONE);
		return (base + pg);
	}
	mprotect(base + pages * pg, pg, PROT_NONE);
	return (base + pages * pg - n);
}

char	*t_gstr(const char *s)
{
	size_t	n;
	char	*p;

	n = strlen(s) + 1;
	p = guard_alloc(n, 0);
	memcpy(p, s, n);
	return (p);
}

char	*t_gstr_front(const char *s)
{
	size_t	n;
	char	*p;

	n = strlen(s) + 1;
	p = guard_alloc(n, 1);
	memcpy(p, s, n);
	return (p);
}

void	*t_gmem(const void *src, size_t n)
{
	void	*p;

	p = guard_alloc(n, 0);
	if (src)
		memcpy(p, src, n);
	else
		memset(p, 0xAA, n);
	return (p);
}

void	*t_gmem_front(const void *src, size_t n)
{
	void	*p;

	p = guard_alloc(n, 1);
	if (src)
		memcpy(p, src, n);
	else
		memset(p, 0xAA, n);
	return (p);
}

/*
** ======================================================================
**  FD CAPTURE
** ======================================================================
*/

int	t_capture_open(void)
{
	char	path[] = "/tmp/libft_tester_XXXXXX";
	int		fd;

	fd = mkstemp(path);
	if (fd >= 0)
		unlink(path);
	return (fd);
}

size_t	t_capture_read(int fd, char *buf, size_t size)
{
	ssize_t	r;
	size_t	n;

	n = 0;
	lseek(fd, 0, SEEK_SET);
	while (n < size && (r = read(fd, buf + n, size - n)) > 0)
		n += (size_t)r;
	(void)!ftruncate(fd, 0);
	lseek(fd, 0, SEEK_SET);
	return (n);
}

/*
** ======================================================================
**  PARENT SIDE: running tests
** ======================================================================
*/

static int			g_total_fails;
static int			g_total_warns;
static const char	*g_filter;
static char			g_failed_names[256][96];
static int			g_nfailed_names;

typedef struct s_res
{
	char	out[16384];
	size_t	len;
	int		status;
	int		crashed;
	int		failed;
	long	calls;
	int		injected;
}	t_res;

static const char	*sig_name(int sig)
{
	if (sig == SIGSEGV)
		return ("SEGFAULT");
	if (sig == SIGBUS)
		return ("BUS ERROR");
	if (sig == SIGABRT)
		return ("ABORT (double free / invalid free / heap corruption)");
	if (sig == SIGALRM)
		return ("TIMEOUT (infinite loop or far too slow)");
	if (sig == SIGFPE)
		return ("FLOATING POINT EXCEPTION");
	if (sig == SIGILL)
		return ("ILLEGAL INSTRUCTION");
	if (sig == SIGKILL)
		return ("KILLED (out of memory?)");
	return ("KILLED BY A SIGNAL");
}

static void	on_signal(int sig)
{
	emit_crash(sig_name(sig));
	_exit(100 + sig);
}

static void	child_setup_signals(void)
{
	static char			altstack[1 << 16];
	stack_t				ss;
	struct sigaction	sa;
	int					sigs[] = {SIGSEGV, SIGBUS, SIGABRT, SIGALRM, SIGFPE, SIGILL};
	size_t				i;

	ss.ss_sp = altstack;
	ss.ss_size = sizeof(altstack);
	ss.ss_flags = 0;
	sigaltstack(&ss, NULL);
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_signal;
	sa.sa_flags = SA_ONSTACK;
	sigemptyset(&sa.sa_mask);
	i = 0;
	while (i < sizeof(sigs) / sizeof(*sigs))
		sigaction(sigs[i++], &sa, NULL);
	signal(SIGPIPE, SIG_IGN);
}

static void	child_main(int out, t_testfn fn, int timeout, long fail_at)
{
	char	msg[64];

	g_out = out;
	g_fail_at = fail_at;
	g_calls = 0;
	g_injected = 0;
	g_nfails = 0;
	g_step[0] = 0;
	child_setup_signals();
	track_init();
	alarm((unsigned)timeout);
	g_track = 1;
	fn();
	g_armed = 0;
	g_step[0] = 0;
	final_heap_check();
	g_track = 0;
	snprintf(msg, sizeof(msg), "%ld", g_calls);
	emit('N', msg);
	snprintf(msg, sizeof(msg), "%d", g_injected);
	emit('I', msg);
	_exit(g_nfails ? 1 : 0);
}

static void	spawn(t_testfn fn, int timeout, long fail_at, t_res *r)
{
	int		fds[2];
	pid_t	pid;
	ssize_t	n;
	char	junk[4096];
	char	*line;
	char	*nl;

	memset(r, 0, sizeof(*r));
	if (pipe(fds) < 0)
	{
		perror("pipe");
		exit(1);
	}
	fflush(stdout);
	fflush(stderr);
	pid = fork();
	if (pid < 0)
	{
		perror("fork");
		exit(1);
	}
	if (pid == 0)
	{
		close(fds[0]);
		child_main(fds[1], fn, timeout, fail_at);
	}
	close(fds[1]);
	while (1)
	{
		if (r->len < sizeof(r->out) - 1)
			n = read(fds[0], r->out + r->len, sizeof(r->out) - 1 - r->len);
		else
			n = read(fds[0], junk, sizeof(junk));
		if (n < 0 && errno == EINTR)
			continue ;
		if (n <= 0)
			break ;
		if (r->len < sizeof(r->out) - 1)
			r->len += (size_t)n;
	}
	close(fds[0]);
	while (waitpid(pid, &r->status, 0) < 0 && errno == EINTR)
		;
	r->out[r->len] = 0;
	line = r->out;
	while (*line)
	{
		nl = strchr(line, '\n');
		if (nl)
			*nl = 0;
		if (line[0] == 'N' && line[1] == '|')
			r->calls = atol(line + 2);
		else if (line[0] == 'I' && line[1] == '|')
			r->injected = atoi(line + 2);
		else if (line[0] == 'C')
			r->crashed = 1;
		if (!nl)
			break ;
		*nl = '\n';
		line = nl + 1;
	}
	if (WIFSIGNALED(r->status) || (WIFEXITED(r->status) && WEXITSTATUS(r->status) >= 98))
		r->crashed = 1;
	if (r->crashed || !WIFEXITED(r->status) || WEXITSTATUS(r->status) != 0)
		r->failed = 1;
}

/* print every F/C/H/L line of a result */
static void	print_details(t_res *r, const char *prefix)
{
	char	*line;
	char	*nl;
	int		printed;

	printed = 0;
	line = r->out;
	while (*line)
	{
		nl = strchr(line, '\n');
		if (nl)
			*nl = 0;
		if (line[1] == '|' && (line[0] == 'F' || line[0] == 'C' || line[0] == 'H' || line[0] == 'L'))
		{
			printf("      %s%s%s%s\n", line[0] == 'C' ? C_RED : "", prefix, line + 2, C_RST);
			printed++;
		}
		if (!nl)
			break ;
		*nl = '\n';
		line = nl + 1;
	}
	if (WIFSIGNALED(r->status) && !printed)
		printf("      " C_RED "%s%s" C_RST "\n", prefix, sig_name(WTERMSIG(r->status)));
}

static void	print_name(const char *name)
{
	int	len;

	len = printf("  %s ", name);
	while (len++ < 72)
		putchar('.');
}

static void	record(const char *name, int level)
{
	if (level == T_WARN)
	{
		g_total_warns++;
		return ;
	}
	g_total_fails++;
	if (g_nfailed_names < 256)
		snprintf(g_failed_names[g_nfailed_names++], 96, "%s", name);
}

void	t_run(const char *name, t_testfn fn, int level, int timeout)
{
	t_res	r;

	if (!t_filter_match(name))
		return ;
	print_name(name);
	spawn(fn, timeout, -1, &r);
	if (!r.failed)
		printf(" " C_GRN "[OK]" C_RST "\n");
	else
	{
		printf(" %s\n", level == T_WARN ? C_YEL "[WARN]" C_RST : C_RED "[KO]" C_RST);
		print_details(&r, "✗ ");
		record(name, level);
	}
}

void	t_run_malloc_fail(const char *name, t_testfn fn, int timeout)
{
	t_res	r;
	t_res	first;
	long	n;
	long	i;
	long	bad;
	long	first_bad;
	char	prefix[80];

	if (!t_filter_match(name))
		return ;
	print_name(name);
	spawn(fn, timeout, -1, &r);
	if (r.failed)
	{
		printf(" " C_RED "[KO]" C_RST "\n");
		print_details(&r, "✗ (normal run, no malloc failing) ");
		record(name, T_MUST);
		return ;
	}
	n = r.calls;
	if (n == 0)
	{
		printf(" " C_RED "[KO]" C_RST "\n      ✗ no call to malloc was seen\n");
		record(name, T_MUST);
		return ;
	}
	bad = 0;
	first_bad = -1;
	i = 0;
	while (i < n)
	{
		spawn(fn, timeout, i, &r);
		if (r.failed)
		{
			if (first_bad < 0)
			{
				first_bad = i;
				first = r;
			}
			bad++;
		}
		i++;
	}
	if (!bad)
	{
		printf(" " C_GRN "[OK]" C_RST C_DIM " %ld/%ld malloc failure points survived" C_RST "\n", n, n);
		return ;
	}
	printf(" " C_RED "[KO]" C_RST " %ld/%ld malloc failure points broken\n", bad, n);
	snprintf(prefix, sizeof(prefix), "✗ [malloc #%ld of %ld returns NULL] ", first_bad + 1, n);
	print_details(&first, prefix);
	record(name, T_MUST);
}

void	t_section(const char *title)
{
	if (g_filter)
		printf(C_CYN "\n --- %s ---\n" C_RST, title);
	else
		TITLE(title);
}

void	t_set_filter(const char *filter)
{
	g_filter = filter;
}

int	t_filter_match(const char *name)
{
	return (!g_filter || strstr(name, g_filter) != NULL);
}

int	t_total_fails(void)
{
	return (g_total_fails);
}

int	t_total_warns(void)
{
	return (g_total_warns);
}

void	t_print_summary(void)
{
	int	i;

	if (!g_nfailed_names)
		return ;
	printf(C_RED "\n\n ========= FAILED TESTS =========== \n\n" C_RST);
	i = 0;
	while (i < g_nfailed_names)
		printf("   ✗ %s\n", g_failed_names[i++]);
	if (g_total_fails > g_nfailed_names)
		printf("   ... and %d more\n", g_total_fails - g_nfailed_names);
	printf(C_DIM "\n   Re-run a single test with:  ./tester <part of its name>\n" C_RST);
}
