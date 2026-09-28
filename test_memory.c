#include "tester.h"

/*
** ALL TOGETHER
** Tests that use many libft functions at once (the other tests each check
** a single function):
**   - one chain piping every allocating function into the next, on guarded
**     inputs, with a malloc failure injected at every single point;
**   - thousands of calls in a row, so a leak hidden in one branch adds up.
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

static void	*map_dup(void *p)
{
	char	*r;

	g_f_calls++;
	t_disarm();
	r = dupstr(p);
	t_arm();
	return (r);
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
/* One-shot chain                                                     */
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
/* Repetition: a leak that only happens in some branch adds up        */
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

static void	chain_ok(void)
{
	char	*s;
	char	**arr;
	char	*j;
	t_list	*lst;
	t_list	*m;
	int		i;

	CASE("ft_strtrim(ft_substr(...)) -> ft_split -> list -> ft_lstmap -> ft_strjoin");
	s = ft_strtrim(t_gstr("  --one two three--  "), t_gstr_front(" -"));
	EXPECT(s && !strcmp(s, "one two three"), "ft_strtrim gave \"%s\"", t_esc(s));
	arr = ft_split(s ? s : "one two three", ' ');
	lst = NULL;
	i = 0;
	while (t_is_block(arr) && t_is_block(arr[i]))
		ft_lstadd_back(&lst, ft_lstnew(arr[i++]));
	EXPECT(i == 3 && ft_lstsize(lst) == 3, "expected 3 words / nodes");
	m = ft_lstmap(lst, map_dup, del_free);
	j = ft_strjoin(m && m->content ? m->content : "", ft_lstlast(lst) ? ft_lstlast(lst)->content : "");
	EXPECT(j && !strcmp(j, "onethree"), "expected \"onethree\", got \"%s\"", t_esc(j));
	ft_lstclear(&m, free);
	ft_lstclear(&lst, nothing);
	EXPECT(m == NULL && lst == NULL, "ft_lstclear must set the lists to NULL");
	t_free_split(arr);
	t_free(s);
	t_free(j);
}

static const t_test	g_all[] = {
	TEST("a chain of functions, no failure", "trim -> split -> lstnew / lstadd_back -> lstmap -> strjoin \
-> lstclear: the functions work together and nothing leaks.", chain_ok),
	{"killer chain, every malloc fails once", "substr -> strtrim -> strjoin -> split -> itoa -> strdup -> \
strmapi -> calloc -> lstnew -> lstmap -> lstclear on guarded inputs. The chain is run once per malloc, \
with that malloc failing: every function must return NULL, free what it allocated and never crash.",
		killer_chain, T_MUST, 20, 1},
	TEST_SLOW("5000 x every allocating function", "substr, strjoin, strtrim, split, itoa, strmapi, strdup \
5000 times each with varying inputs: a leak in a single branch adds up.", repeat_everything),
};

void	run_memory(void)
{
	t_section("ALL TOGETHER");
	GROUP_OTHER("all together", g_all);
}
