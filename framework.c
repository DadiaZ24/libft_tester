#include "tester.h"
#include <signal.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/resource.h>

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

/* async-signal-safe version, used from the signal handler.
** 'C' = crash, 'M' = called a function missing from libft.a */
static void	emit_signal(char type, const char *what)
{
	char	buf[800];
	size_t	n;
	size_t	i;

	n = 0;
	buf[n++] = type;
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

static void	emit_crash(const char *what)
{
	emit_signal('C', what);
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
** Every test runs in a forked child. What it reports (and anything your
** functions printed) goes to the trace file, never to the screen.
*/

static FILE			*g_trace;
static const char	*g_trace_path = "traces.log";
static int			g_cap = -1;

/* what a child writes to stdout/stderr (your printf, glibc messages...) */
#define CAP_MAX		2048
/* so a function printing in an infinite loop can't fill the disk */
#define CAP_FSIZE	((rlim_t)16 << 20)

typedef struct s_res
{
	char	out[16384];
	size_t	len;
	char	cap[CAP_MAX];
	size_t	caplen;
	int		status;
	int		crashed;
	int		missing;
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

/* stub of a function missing from libft.a (libft_api.c) */
void	t_missing_call(const char *fn)
{
	char	msg[128];

	snprintf(msg, sizeof(msg), "called %s, which is not in your libft.a", fn);
	emit_signal('M', msg);
	_exit(97);
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
	signal(SIGXFSZ, SIG_IGN);
}

/* stdout/stderr of the child go to g_cap, never to the screen */
static void	child_redirect_output(void)
{
	struct rlimit	rl;

	if (g_cap < 0)
		return ;
	dup2(g_cap, STDOUT_FILENO);
	dup2(g_cap, STDERR_FILENO);
	rl.rlim_cur = CAP_FSIZE;
	rl.rlim_max = CAP_FSIZE;
	setrlimit(RLIMIT_FSIZE, &rl);
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
	child_redirect_output();
	child_setup_signals();
	track_init();
	alarm((unsigned)timeout);
	g_track = 1;
	fn();
	g_armed = 0;
	g_step[0] = 0;
	final_heap_check();
	g_track = 0;
	fflush(stdout);
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
	if (g_cap < 0)
		g_cap = t_capture_open();
	if (pipe(fds) < 0)
	{
		perror("pipe");
		exit(1);
	}
	fflush(stdout);
	fflush(stderr);
	if (g_trace)
		fflush(g_trace);
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
	if (g_cap >= 0)
		r->caplen = t_capture_read(g_cap, r->cap, sizeof(r->cap) - 1);
	r->cap[r->caplen] = 0;
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
		else if (line[0] == 'M')
			r->missing = 1;
		if (!nl)
			break ;
		*nl = '\n';
		line = nl + 1;
	}
	if (WIFSIGNALED(r->status) || (WIFEXITED(r->status) && WEXITSTATUS(r->status) >= 98))
		r->crashed = 1;
	if (r->crashed || r->missing || !WIFEXITED(r->status) || WEXITSTATUS(r->status) != 0)
		r->failed = 1;
}

/*
** ----------------------------------------------------------------------
**  Trace file
** ----------------------------------------------------------------------
*/

void	t_trace_open(void)
{
	const char	*env;

	env = getenv("LIBFT_TESTER_TRACE");
	if (env && *env)
		g_trace_path = env;
	g_trace = fopen(g_trace_path, env && *env ? "a" : "w");
	t_trace("\n==================== TESTS ====================\n");
}

void	t_trace(const char *fmt, ...)
{
	va_list	ap;

	if (!g_trace)
		return ;
	va_start(ap, fmt);
	vfprintf(g_trace, fmt, ap);
	va_end(ap);
}

/* what the functions printed, indented */
static void	trace_output(t_res *r)
{
	char	*line;
	char	*nl;

	if (!r->caplen)
		return ;
	t_trace("              output of your functions (stdout/stderr)%s:\n",
		r->caplen >= CAP_MAX - 1 ? ", first 2 KB" : "");
	line = r->cap;
	while (*line)
	{
		nl = strchr(line, '\n');
		if (nl)
			*nl = 0;
		t_trace("              | %s\n", line);
		if (!nl)
			break ;
		*nl = '\n';
		line = nl + 1;
	}
}

/* every F/C/H/L/M line of a result */
static void	trace_details(t_res *r, const char *prefix)
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
		if (line[0] && line[1] == '|' && strchr("FCHLM", line[0]))
		{
			t_trace("              %s%s\n", prefix, line + 2);
			printed++;
		}
		if (!nl)
			break ;
		*nl = '\n';
		line = nl + 1;
	}
	if (WIFSIGNALED(r->status) && !printed)
		t_trace("              %s%s\n", prefix, sig_name(WTERMSIG(r->status)));
	trace_output(r);
}

/*
** ----------------------------------------------------------------------
**  Results, grouped by libft function
** ----------------------------------------------------------------------
** Screen: one line per function, one mark per test
**           ✔ passed   ✘ failed   ! undefined behaviour not handled (WARN)
**           ? calls a function that is missing from libft.a
**         or, with -v, one line per test.
** Trace:  every test; for one that fails, what it checks and why it failed.
*/

enum e_verdict
{
	V_OK,
	V_KO,
	V_WARN,
	V_MISSING
};

typedef struct s_stats
{
	int		tests[4];
	int		f_ok;
	int		f_ko;
	int		f_missing;
}	t_stats;

#define NAME_COLS	16
#define MARK_COLS	14

static t_stats		g_st;
static const char	*g_filter;
static int			g_verbose;
static char			g_failed[80][160];
static int			g_nfailed;

static const char	*g_tags[] = {"[OK]", "[KO]", "[WARN]", "[MISSING]"};
static const char	*g_colors[] = {C_GRN, C_RED, C_YEL, C_MAG};
static const char	*g_marks[] = {"✔", "✘", "!", "?"};

static void	section_start(void);

/* "label text..." wrapped at ~100 columns, aligned after the label */
static void	trace_wrap(const char *label, const char *text)
{
	size_t	col;
	size_t	w;
	size_t	indent;

	indent = 14 + strlen(label) + 1;
	t_trace("              %s ", label);
	col = indent;
	while (*text)
	{
		w = strcspn(text, " ");
		if (col + w > 100 && col > indent)
		{
			t_trace("\n%*s", (int)indent, "");
			col = indent;
		}
		t_trace("%.*s", (int)w, text);
		col += w;
		text += w;
		while (*text == ' ')
			text++;
		if (*text && col + 1 <= 100)
		{
			t_trace(" ");
			col++;
		}
	}
	t_trace("\n");
}

static int	trace_result(const t_test *t, int num, int v, t_res *r,
	const char *prefix, const char *note)
{
	t_trace("  %-9s %2d. %s\n", g_tags[v], num, t->name);
	if (v != V_OK)
		trace_wrap("what:", t->why);
	if (note)
		t_trace("              %s\n", note);
	if (r && v == V_OK)
		trace_output(r);
	else if (r)
		trace_details(r, prefix);
	if (v != V_OK)
		t_trace("\n");
	return (v);
}

/* malloc failure injection: once normally to count the mallocs, then once
** per malloc with exactly that one returning NULL */
static int	run_malloc_fail(const t_test *t, int num, t_res *r)
{
	t_res	first;
	long	n;
	long	i;
	long	bad;
	long	first_bad;
	char	prefix[96];
	char	note[128];

	n = r->calls;
	if (n == 0)
		return (trace_result(t, num, V_KO, NULL, "", "✗ no call to malloc was seen"));
	bad = 0;
	first_bad = -1;
	i = 0;
	while (i < n)
	{
		spawn(t->fn, t->timeout, i, r);
		if (r->failed && first_bad < 0)
		{
			first_bad = i;
			first = *r;
		}
		bad += r->failed;
		i++;
	}
	if (!bad)
	{
		snprintf(note, sizeof(note), "%ld/%ld malloc failure points survived", n, n);
		return (trace_result(t, num, V_OK, NULL, "", note));
	}
	snprintf(note, sizeof(note), "%ld/%ld malloc failure points broken, the first one:", bad, n);
	snprintf(prefix, sizeof(prefix), "✗ [malloc #%ld of %ld returns NULL] ", first_bad + 1, n);
	return (trace_result(t, num, V_KO, &first, prefix, note));
}

static int	run_test(const t_test *t, int num)
{
	t_res	r;

	spawn(t->fn, t->timeout, -1, &r);
	if (r.missing)
		return (trace_result(t, num, V_MISSING, &r, "✗ ", NULL));
	if (r.failed)
		return (trace_result(t, num, t->level == T_WARN ? V_WARN : V_KO, &r,
				t->mfail ? "✗ (normal run, no malloc failing) " : "✗ ", NULL));
	if (t->mfail)
		return (run_malloc_fail(t, num, &r));
	return (trace_result(t, num, V_OK, &r, "", NULL));
}

static int	test_selected(const char *group, const t_test *t)
{
	return (!g_filter || strstr(group, g_filter) || strstr(t->name, g_filter));
}

static void	pad_marks(int used)
{
	while (used++ < MARK_COLS)
		printf("  ");
}

/* the function is not in libft.a: its tests are not even run */
static void	group_missing(const char *fn, const t_test *tests, size_t n)
{
	size_t	i;
	int		declared;

	t_func_exists(fn, &declared);
	t_trace("  [MISSING] %s is not in your libft.a%s\n", fn,
		declared ? "" : " (not declared in libft.h either)");
	t_trace("            (not written yet, or not in the SRCS of your Makefile)\n");
	t_trace("            these %zu tests were not run:\n", n);
	i = 0;
	while (i < n)
	{
		t_trace("              %2zu. %s\n", i + 1, tests[i].name);
		i++;
	}
	if (g_verbose)
		printf("\n  %s\n     " C_MAG "[MISSING]" C_RST " %zu tests not run\n", fn, n);
	else
	{
		printf("  %-*s ", NAME_COLS, fn);
		i = 0;
		while (i++ < n)
			printf(C_DIM "· " C_RST);
		pad_marks((int)n);
		printf(" %2d/%-2zu " C_MAG "[MISSING]" C_RST "\n", 0, n);
	}
	g_st.tests[V_MISSING] += (int)n;
	g_st.f_missing++;
	if (g_nfailed < 80)
		snprintf(g_failed[g_nfailed++], 160, "%s  (missing)", fn);
}

static void	group_end(const char *fn, int is_libft, int *v, size_t n)
{
	char	line[160];
	size_t	i;
	int		ran;
	int		ok;
	int		bad;
	size_t	len;

	ran = 0;
	ok = 0;
	bad = 0;
	len = (size_t)snprintf(line, 160, "%s  (tests", fn);
	i = 0;
	while (i < n)
	{
		if (v[i] >= 0)
			ran++;
		if (v[i] == V_OK)
			ok++;
		if (v[i] == V_KO || v[i] == V_MISSING)
		{
			bad++;
			if (len < 150)
				len += (size_t)snprintf(line + len, 160 - len, " %zu", i + 1);
		}
		i++;
	}
	if (bad && g_nfailed < 80)
		snprintf(g_failed[g_nfailed++], 160, "%s)", line);
	if (is_libft && bad)
		g_st.f_ko++;
	else if (is_libft)
		g_st.f_ok++;
	if (g_verbose)
		printf("     -> %d/%d passed  %s%s" C_RST "\n", ok, ran, bad ? C_RED : C_GRN, bad ? "[KO]" : "[OK]");
	else
	{
		pad_marks(ran);
		printf(" %2d/%-2d %s%s" C_RST "\n", ok, ran, bad ? C_RED : C_GRN, bad ? "[KO]" : "[OK]");
	}
	t_trace("  -> %s: %d/%d tests passed\n", fn, ok, ran);
}

void	t_group(const char *fn, int is_libft, const t_test *tests, size_t n)
{
	int		v[64];
	size_t	i;
	size_t	sel;
	int		len;

	sel = 0;
	i = 0;
	while (i < n)
		sel += test_selected(fn, &tests[i++]);
	if (!sel || n > 64)
		return ;
	section_start();
	t_trace("\n==================== %s ====================\n\n", fn);
	if (is_libft && !t_func_exists(fn, NULL))
		return (group_missing(fn, tests, n));
	if (g_verbose)
		printf("\n  %s\n", fn);
	else
		printf("  %-*s ", NAME_COLS, fn);
	i = 0;
	while (i < n)
	{
		v[i] = -1;
		if (test_selected(fn, &tests[i]))
		{
			if (g_verbose)
			{
				len = printf("     %2zu. %s ", i + 1, tests[i].name);
				while (len++ < 80)
					putchar('.');
			}
			v[i] = run_test(&tests[i], (int)i + 1);
			g_st.tests[v[i]]++;
			if (g_verbose)
				printf(" %s%s" C_RST "\n", g_colors[v[i]], g_tags[v[i]]);
			else
				printf("%s%s" C_RST " ", g_colors[v[i]], g_marks[v[i]]);
			fflush(stdout);
		}
		i++;
	}
	group_end(fn, is_libft, v, n);
}

/* the title is printed with the first group of the section that runs, so
** a filtered run shows no empty sections */
static const char	*g_section;

void	t_section(const char *title)
{
	g_section = title;
}

static void	section_start(void)
{
	if (!g_section)
		return ;
	TITLE(g_section);
	t_trace("\n\n######## %s ########\n", g_section);
	g_section = NULL;
}

void	t_set_filter(const char *filter)
{
	g_filter = filter;
}

void	t_set_verbose(int verbose)
{
	g_verbose = verbose;
}

int	t_total_fails(void)
{
	return (g_st.tests[V_KO]);
}

int	t_total_missing(void)
{
	return (g_st.tests[V_MISSING]);
}

void	t_print_summary(void)
{
	int	i;

	if (g_nfailed)
	{
		t_trace("\n==================== NOT PASSED ====================\n\n");
		i = 0;
		while (i < g_nfailed)
			t_trace("  ✗ %s\n", g_failed[i++]);
	}
	printf(C_CYN "\n ========= RESULT =========== \n\n" C_RST);
	printf("  functions  " C_GRN "%3d OK" C_RST "   " C_RED "%3d KO" C_RST "   " C_MAG "%3d MISSING" C_RST "\n",
		g_st.f_ok, g_st.f_ko, g_st.f_missing);
	printf("  tests      " C_GRN "%3d OK" C_RST "   " C_RED "%3d KO" C_RST "   " C_MAG "%3d MISSING" C_RST
		"   " C_YEL "%d WARN" C_RST C_DIM " (undefined behaviour, not an error)" C_RST "\n",
		g_st.tests[V_OK], g_st.tests[V_KO], g_st.tests[V_MISSING], g_st.tests[V_WARN]);
	if (!g_st.tests[V_KO] && !g_st.tests[V_MISSING])
		printf("\n  🎉 " C_GRN "ALL THE TESTS PASSED! MAY THE FORCE BE WITH YOU" C_RST "\n");
	printf(C_DIM "\n  ✔ passed  ✘ failed  ! undefined behaviour  ? calls a missing function"
		"\n  What each test checks and why it failed: %s"
		"\n  One line per test: make run V=1    Only some tests: make run T=<name>" C_RST "\n\n",
		g_trace_path);
	if (g_trace)
		fclose(g_trace);
	g_trace = NULL;
}
