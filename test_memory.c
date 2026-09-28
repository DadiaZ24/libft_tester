#include "tester.h"

/*
** MEMORY KILLERS
**
** 1. Malloc failure injection: each scenario is run once normally to count
**    its mallocs, then once per malloc with exactly that malloc returning
**    NULL. At every single failure point the function must:
**      - not crash (no segfault / double free),
**      - return NULL (the subject: "NULL if the allocation fails"),
**      - free everything it had already allocated (no leak),
**      - for ft_lstmap: del() the content f() produced for the node that
**        could not be allocated, and leave the original list untouched.
**    Any one of these, hit during an evaluation, is a 0.
**
** 2. One-shot chain: one scenario that pipes every allocating function into
**    the next, on guarded inputs, with the same failure injection.
**
** 3. Very long lists (1 000 000 nodes): recursive ft_lstsize / ft_lstclear /
**    ft_lstmap blow up the stack. Reported as WARN.
*/

static int	g_del_calls;
static int	g_f_calls;

static char	*dupstr(const char *s)
{
	size_t	n;
	char	*r;

	n = strlen(s) + 1;
	r = malloc(n);
	if (r)
		memcpy(r, s, n);
	return (r);
}

static void	del_free(void *p)
{
	g_del_calls++;
	free(p);
}

static void	del_noop(void *p)
{
	(void)p;
	g_del_calls++;
}

static void	*map_dup(void *p)
{
	char	*r;

	g_f_calls++;
	t_disarm();
	r = dupstr(p);
	t_arm();
	return (r);
}

static void	*map_same(void *p)
{
	g_f_calls++;
	return (p);
}

static char	map_rot(unsigned int i, char c)
{
	return (c + (char)(i % 3));
}

static void	nothing(void *p)
{
	(void)p;
}

/* check the result of one armed call */
static void	after(const void *res, int inj_before, const char *fn)
{
	if (t_injected() && !inj_before)
		EXPECT(res == NULL, "%s returned non-NULL although one of its mallocs returned NULL", fn);
}

/* ------------------------------------------------------------------ */
/* 1. Malloc failure injection, one function at a time                */
/* ------------------------------------------------------------------ */

#define SIMPLE_STR_MF(NAME, LABEL, CALL, EXP) \
static void	NAME(void) \
{ \
	char	*r; \
\
	CASE("%s", LABEL); \
	t_arm(); \
	r = CALL; \
	t_disarm(); \
	if (t_injected()) \
	{ \
		EXPECT(r == NULL, "returned non-NULL although its malloc returned NULL"); \
		t_free(r); \
	} \
	else \
		t_check_str(LABEL, r, EXP); \
}

SIMPLE_STR_MF(mf_strdup, "ft_strdup(\"hello\")", ft_strdup(t_gstr("hello")), "hello")
SIMPLE_STR_MF(mf_substr, "ft_substr(\"Hello World\", 6, 5)", ft_substr(t_gstr("Hello World"), 6, 5), "World")
SIMPLE_STR_MF(mf_substr_empty, "ft_substr(\"hola\", 42, 3)", ft_substr(t_gstr("hola"), 42, 3), "")
SIMPLE_STR_MF(mf_strjoin, "ft_strjoin(\"Hello\", \" World\")", ft_strjoin(t_gstr("Hello"), t_gstr(" World")), "Hello World")
SIMPLE_STR_MF(mf_strtrim, "ft_strtrim(\"  hello  \", \" \")", ft_strtrim(t_gstr("  hello  "), t_gstr(" ")), "hello")
SIMPLE_STR_MF(mf_strtrim_all, "ft_strtrim(\"xxxx\", \"x\")", ft_strtrim(t_gstr("xxxx"), t_gstr("x")), "")
SIMPLE_STR_MF(mf_itoa, "ft_itoa(-2147483648)", ft_itoa(INT_MIN), "-2147483648")
SIMPLE_STR_MF(mf_itoa0, "ft_itoa(0)", ft_itoa(0), "0")
SIMPLE_STR_MF(mf_strmapi, "ft_strmapi(\"abcdef\", f)", ft_strmapi(t_gstr("abcdef"), map_rot), "acedfh")

static void	mf_calloc(void)
{
	void	*r;

	CASE("ft_calloc(10, sizeof(int))");
	t_arm();
	r = ft_calloc(10, sizeof(int));
	t_disarm();
	if (t_injected())
		EXPECT(r == NULL, "returned non-NULL although its malloc returned NULL");
	else
		EXPECT(r && t_is_block(r), "expected a malloc'd block");
	t_free(r);
}

static void	mf_calloc0(void)
{
	void	*r;

	CASE("ft_calloc(0, 0)");
	t_arm();
	r = ft_calloc(0, 0);
	t_disarm();
	if (t_injected())
		EXPECT(r == NULL, "returned non-NULL although its malloc returned NULL");
	t_free(r);
}

static void	split_scenario(const char *s, char c)
{
	char	**r;
	size_t	i;

	CASE("ft_split(\"%s\", '%c')", t_esc(s), c);
	t_arm();
	r = ft_split(t_gstr(s), c);
	t_disarm();
	if (t_injected())
		EXPECT(r == NULL, "returned non-NULL although one of its mallocs returned NULL"
			" (the subject: NULL if ANY allocation fails)");
	else if (!r)
		t_fail("returned NULL");
	else
	{
		i = 0;
		while (t_is_block(r[i]))
			i++;
		EXPECT(r[i] == NULL, "the array is not NULL-terminated");
	}
	t_free_split(r);
}

static void	mf_split1(void) { split_scenario("hello world foo bar", ' '); }
static void	mf_split2(void) { split_scenario(",,a,,b,,", ','); }
static void	mf_split3(void) { split_scenario("single", ' '); }
static void	mf_split4(void) { split_scenario("   ", ' '); }
static void	mf_split5(void) { split_scenario("  lorem ipsum dolor sit amet consectetur adipiscing  ", ' '); }

static void	mf_lstnew(void)
{
	t_list	*n;

	CASE("ft_lstnew(\"x\")");
	t_arm();
	n = ft_lstnew("x");
	t_disarm();
	if (t_injected())
		EXPECT(n == NULL, "returned non-NULL although its malloc returned NULL");
	t_free(n);
}

static void	lstmap_scenario(int len)
{
	static const char	*w[] = {"a", "bb", "ccc", "dddd", "eeeee", "ffffff"};
	t_list				*l;
	t_list				*last;
	t_list				*node;
	t_list				*m;
	t_list				*cur;
	int					i;
	int					ok;

	l = NULL;
	last = NULL;
	i = 0;
	while (i < len)
	{
		node = ft_lstnew(dupstr(w[i]));
		if (last)
			last->next = node;
		else
			l = node;
		last = node;
		i++;
	}
	g_del_calls = 0;
	g_f_calls = 0;
	CASE("ft_lstmap(<%d nodes>, strdup, del)", len);
	t_arm();
	m = ft_lstmap(l, map_dup, del_free);
	t_disarm();
	if (t_injected())
		EXPECT(m == NULL, "returned non-NULL although one of its mallocs returned NULL");
	else
		EXPECT(m != NULL && ft_lstsize(m) == (unsigned)len, "wrong result");
	ok = 1;
	cur = l;
	i = 0;
	while (cur && ok)
	{
		ok = t_is_block(cur) && t_is_block(cur->content) && strcmp(cur->content, w[i++]) == 0;
		cur = cur->next;
	}
	EXPECT(ok && i == len, "the ORIGINAL list was freed or modified");
	while (m)
	{
		cur = m->next;
		if (t_is_block(m))
		{
			t_free(m->content);
			free(m);
		}
		m = cur;
	}
	while (l)
	{
		cur = l->next;
		free(l->content);
		free(l);
		l = cur;
	}
}

static void	mf_lstmap1(void) { lstmap_scenario(1); }
static void	mf_lstmap2(void) { lstmap_scenario(2); }
static void	mf_lstmap6(void) { lstmap_scenario(6); }

/* ------------------------------------------------------------------ */
/* 2. One-shot chain                                                  */
/* ------------------------------------------------------------------ */

static void	killer_chain(void)
{
	char	*s1;
	char	*s2;
	char	*s3;
	char	**arr;
	char	*num;
	char	*dup;
	char	*mapped;
	void	*z0;
	void	*z1;
	t_list	*lst;
	t_list	*node;
	t_list	*m;
	int		inj;
	int		i;

	lst = NULL;
	inj = t_injected();
	CASE("chain: ft_substr(\"  --Hello, 42 World!--  \", 0, SIZE_MAX)");
	t_arm();
	s1 = ft_substr(t_gstr("  --Hello, 42 World!--  "), 0, SIZE_MAX);
	t_disarm();
	after(s1, inj, "ft_substr");
	inj = t_injected();
	CASE("chain: ft_strtrim(s1, \" -\")");
	t_arm();
	s2 = ft_strtrim(s1 ? s1 : t_gstr_front("--x--"), t_gstr_front(" -"));
	t_disarm();
	after(s2, inj, "ft_strtrim");
	inj = t_injected();
	CASE("chain: ft_strjoin(s2, \" and more words\")");
	t_arm();
	s3 = ft_strjoin(s2 ? s2 : t_gstr("x"), t_gstr_front(" and more words"));
	t_disarm();
	after(s3, inj, "ft_strjoin");
	inj = t_injected();
	CASE("chain: ft_split(s3, ' ')");
	t_arm();
	arr = ft_split(s3 ? s3 : t_gstr("a b"), ' ');
	t_disarm();
	after(arr, inj, "ft_split");
	inj = t_injected();
	CASE("chain: ft_itoa(INT_MIN)");
	t_arm();
	num = ft_itoa(INT_MIN);
	t_disarm();
	after(num, inj, "ft_itoa");
	inj = t_injected();
	CASE("chain: ft_strdup(num)");
	t_arm();
	dup = ft_strdup(num ? num : "0");
	t_disarm();
	after(dup, inj, "ft_strdup");
	inj = t_injected();
	CASE("chain: ft_strmapi(dup, f)");
	t_arm();
	mapped = ft_strmapi(dup ? dup : "0", map_rot);
	t_disarm();
	after(mapped, inj, "ft_strmapi");
	inj = t_injected();
	CASE("chain: ft_calloc(0, 0)");
	t_arm();
	z0 = ft_calloc(0, 0);
	t_disarm();
	after(z0, inj, "ft_calloc");
	inj = t_injected();
	CASE("chain: ft_calloc(7, 3)");
	t_arm();
	z1 = ft_calloc(7, 3);
	t_disarm();
	after(z1, inj, "ft_calloc");
	i = 0;
	while (t_is_block(arr) && t_is_block(arr[i]))
	{
		inj = t_injected();
		CASE("chain: ft_lstnew(arr[%d]) + ft_lstadd_back", i);
		t_arm();
		node = ft_lstnew(arr[i]);
		t_disarm();
		after(node, inj, "ft_lstnew");
		if (node)
			ft_lstadd_back(&lst, node);
		i++;
	}
	g_f_calls = 0;
	g_del_calls = 0;
	inj = t_injected();
	CASE("chain: ft_lstmap(list of words, strdup, del)");
	t_arm();
	m = ft_lstmap(lst, map_dup, del_free);
	t_disarm();
	if (lst)
		after(m, inj, "ft_lstmap");
	CASE("chain: ft_lstclear(&mapped, free) + ft_lstclear(&list, nothing)");
	ft_lstclear(&m, free);
	ft_lstclear(&lst, nothing);
	EXPECT(m == NULL && lst == NULL, "ft_lstclear must set the list pointer to NULL");
	CASE("chain: cleanup");
	t_free_split(arr);
	t_free(s1);
	t_free(s2);
	t_free(s3);
	t_free(num);
	t_free(dup);
	t_free(mapped);
	t_free(z0);
	t_free(z1);
}

/* ------------------------------------------------------------------ */
/* 3. Very long lists                                                 */
/* ------------------------------------------------------------------ */

#define DEEP 1000000

static t_list	*build_deep(void)
{
	t_list	*head;
	t_list	*last;
	t_list	*n;
	int		i;

	head = ft_lstnew("deep");
	last = head;
	i = 1;
	while (last && i < DEEP)
	{
		n = ft_lstnew("deep");
		last->next = n;
		last = n;
		i++;
	}
	return (head);
}

static void	free_deep(t_list *l)
{
	t_list	*next;

	while (l)
	{
		next = l->next;
		free(l);
		l = next;
	}
}

static void	deep_size_last(void)
{
	t_list	*l;
	t_list	*last;

	l = build_deep();
	CASE("ft_lstsize(<1 000 000 nodes>)");
	EXPECT(ft_lstsize(l) == DEEP, "wrong size");
	last = l;
	while (last->next)
		last = last->next;
	CASE("ft_lstlast(<1 000 000 nodes>)");
	EXPECT(ft_lstlast(l) == last, "wrong last node");
	g_f_calls = 0;
	CASE("ft_lstiter(<1 000 000 nodes>, f)");
	ft_lstiter(l, (void (*)(void *))nothing);
	free_deep(l);
}

static void	deep_clear(void)
{
	t_list	*l;

	l = build_deep();
	g_del_calls = 0;
	CASE("ft_lstclear(<1 000 000 nodes>, del)");
	ft_lstclear(&l, del_noop);
	EXPECT(l == NULL && g_del_calls == DEEP, "del called %d time(s)", g_del_calls);
}

static void	deep_map(void)
{
	t_list	*l;
	t_list	*m;

	l = build_deep();
	g_f_calls = 0;
	CASE("ft_lstmap(<1 000 000 nodes>, f, del)");
	m = ft_lstmap(l, map_same, del_noop);
	EXPECT(m && g_f_calls == DEEP, "f called %d time(s)", g_f_calls);
	free_deep(m);
	free_deep(l);
}

static void	deep_add_back(void)
{
	t_list	*l;
	t_list	*n;
	int		i;

	l = NULL;
	i = 0;
	CASE("20 000 x ft_lstadd_back");
	while (i < 20000)
	{
		n = ft_lstnew("x");
		ft_lstadd_back(&l, n);
		i++;
	}
	EXPECT(ft_lstsize(l) == 20000, "wrong size");
	free_deep(l);
}

/* ------------------------------------------------------------------ */
/* 4. Repetition: a leak that only happens in some branch adds up     */
/* ------------------------------------------------------------------ */

static void	repeat_everything(void)
{
	int		i;
	char	*r;
	char	**a;

	CASE("5000 x (substr, strjoin, strtrim, split, itoa, strmapi, strdup) + free");
	i = 0;
	while (i < 5000)
	{
		t_free(ft_substr("repeat", i % 8, i % 5));
		t_free(ft_strjoin(i % 2 ? "" : "abc", i % 3 ? "" : "def"));
		t_free(ft_strtrim(i % 2 ? "  x  " : "    ", " "));
		a = ft_split(i % 2 ? " a b  c " : "", ' ');
		t_free_split(a);
		t_free(ft_itoa(i * 7919 - 20000000));
		t_free(ft_strmapi(i % 2 ? "abc" : "", map_rot));
		r = ft_strdup(i % 2 ? "dup" : "");
		t_free(r);
		i++;
	}
}

/* ------------------------------------------------------------------ */

void	run_memory(void)
{
	t_section("MEMORY KILLERS: malloc fails at EVERY point (no crash, NULL, no leak)");
	t_run_malloc_fail("malloc fail: ft_strdup", mf_strdup, 5);
	t_run_malloc_fail("malloc fail: ft_calloc(10, 4)", mf_calloc, 5);
	t_run_malloc_fail("malloc fail: ft_calloc(0, 0)", mf_calloc0, 5);
	t_run_malloc_fail("malloc fail: ft_substr", mf_substr, 5);
	t_run_malloc_fail("malloc fail: ft_substr (start > len)", mf_substr_empty, 5);
	t_run_malloc_fail("malloc fail: ft_strjoin", mf_strjoin, 5);
	t_run_malloc_fail("malloc fail: ft_strtrim", mf_strtrim, 5);
	t_run_malloc_fail("malloc fail: ft_strtrim (everything trimmed)", mf_strtrim_all, 5);
	t_run_malloc_fail("malloc fail: ft_itoa(INT_MIN)", mf_itoa, 5);
	t_run_malloc_fail("malloc fail: ft_itoa(0)", mf_itoa0, 5);
	t_run_malloc_fail("malloc fail: ft_strmapi", mf_strmapi, 5);
	t_run_malloc_fail("malloc fail: ft_split 4 words", mf_split1, 5);
	t_run_malloc_fail("malloc fail: ft_split \",,a,,b,,\"", mf_split2, 5);
	t_run_malloc_fail("malloc fail: ft_split 1 word", mf_split3, 5);
	t_run_malloc_fail("malloc fail: ft_split only delimiters", mf_split4, 5);
	t_run_malloc_fail("malloc fail: ft_split 7 words", mf_split5, 5);
	t_run_malloc_fail("malloc fail: ft_lstnew", mf_lstnew, 5);
	t_run_malloc_fail("malloc fail: ft_lstmap 1 node", mf_lstmap1, 5);
	t_run_malloc_fail("malloc fail: ft_lstmap 2 nodes", mf_lstmap2, 5);
	t_run_malloc_fail("malloc fail: ft_lstmap 6 nodes", mf_lstmap6, 5);
	t_section("MEMORY KILLERS: one-shot chain (everything, guarded, every malloc fails once)");
	t_run_malloc_fail("killer chain: substr > strtrim > strjoin > split > itoa > ... > lstmap", killer_chain, 10);
	t_section("MEMORY KILLERS: repetition and very long lists");
	t_run("5000 x every allocating function (leaks add up)", repeat_everything, T_MUST, 20);
	t_run("20 000 x ft_lstadd_back", deep_add_back, T_MUST, 20);
	t_run("1 000 000 nodes: ft_lstsize / ft_lstlast / ft_lstiter", deep_size_last, T_WARN, 30);
	t_run("1 000 000 nodes: ft_lstclear (recursion = stack overflow)", deep_clear, T_WARN, 30);
	t_run("1 000 000 nodes: ft_lstmap (recursion = stack overflow)", deep_map, T_WARN, 30);
}
