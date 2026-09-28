#include "tester.h"

/*
** PART 3 - Linked list (mandatory in subject v19.3, no longer a bonus)
** Freed nodes are filled with 0xDF by the tester: reading node->next after
** free(node) gives 0xDFDFDFDFDFDFDFDF and crashes immediately.
*/

static int		g_del_calls;
static void		*g_del_seen[16];
static int		g_f_calls;
static void		*g_f_seen[16];

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

static void	del_rec(void *p)
{
	if (g_del_calls < 16)
		g_del_seen[g_del_calls] = p;
	g_del_calls++;
	free(p);
}

static void	del_count_only(void *p)
{
	(void)p;
	g_del_calls++;
}

static void	iter_rec(void *p)
{
	if (g_f_calls < 16)
		g_f_seen[g_f_calls] = p;
	g_f_calls++;
}

/* f for lstmap: its own malloc never fails (only libft's mallocs do) */
static void	*map_upper(void *p)
{
	char	*r;
	size_t	i;

	if (g_f_calls < 16)
		g_f_seen[g_f_calls] = p;
	g_f_calls++;
	t_disarm();
	r = dupstr(p);
	t_arm();
	i = 0;
	while (r && r[i])
	{
		r[i] = toupper((unsigned char)r[i]);
		i++;
	}
	return (r);
}

static const char	*g_words[] = {"zero", "one", "two", "three", "four", "five", "six", "seven"};

/* builds a list with ft_lstnew only (does not depend on ft_lstadd_*) */
static t_list	*build(int n, int alloc_content)
{
	t_list	*head;
	t_list	*last;
	t_list	*node;
	int		i;

	head = NULL;
	last = NULL;
	i = 0;
	while (i < n)
	{
		node = ft_lstnew(alloc_content ? (void *)dupstr(g_words[i % 8]) : (void *)g_words[i % 8]);
		if (!node)
			return (head);
		if (last)
			last->next = node;
		else
			head = node;
		last = node;
		i++;
	}
	return (head);
}

static void	free_list(t_list *l, int free_content)
{
	t_list	*next;

	while (l)
	{
		next = l->next;
		if (free_content)
			free(l->content);
		free(l);
		l = next;
	}
}

/* ------------------------------------------------------------------ */

static void	test_lstnew(void)
{
	t_list	*n;
	int		x;

	CASE("ft_lstnew(\"hello\")");
	n = ft_lstnew("hello");
	if (!n)
	{
		t_fail("returned NULL");
		return ;
	}
	EXPECT(t_is_block(n) && t_block_size(n) >= sizeof(t_list), "node is not a malloc'd block of sizeof(t_list)");
	EXPECT(n->content == (void *)"hello", "content must be the given pointer (not a copy)");
	EXPECT(n->next == NULL, "next must be NULL (the tester's malloc returns garbage on purpose)");
	free(n);
	CASE("ft_lstnew(NULL)");
	n = ft_lstnew(NULL);
	EXPECT(n && n->content == NULL && n->next == NULL, "content and next must be NULL");
	t_free(n);
	CASE("ft_lstnew(&int)");
	n = ft_lstnew(&x);
	EXPECT(n && n->content == &x, "content must be the given pointer");
	t_free(n);
}

static void	test_lstadd_front(void)
{
	t_list	*lst;
	t_list	*a;
	t_list	*b;
	t_list	*c;

	lst = NULL;
	a = ft_lstnew("A");
	b = ft_lstnew("B");
	c = ft_lstnew("C");
	CASE("ft_lstadd_front(&empty, A)");
	ft_lstadd_front(&lst, a);
	EXPECT(lst == a && a->next == NULL, "list must be A -> NULL");
	CASE("ft_lstadd_front(&[A], B)");
	ft_lstadd_front(&lst, b);
	EXPECT(lst == b && b->next == a && a->next == NULL, "list must be B -> A");
	CASE("ft_lstadd_front(&[B, A], C)");
	ft_lstadd_front(&lst, c);
	EXPECT(lst == c && c->next == b && b->next == a && a->next == NULL, "list must be C -> B -> A");
	t_free(a);
	t_free(b);
	t_free(c);
}

static void	test_lstadd_back(void)
{
	t_list	*lst;
	t_list	*n[5];
	t_list	*chain;
	int		i;

	lst = NULL;
	i = 0;
	while (i < 5)
	{
		n[i] = ft_lstnew((void *)g_words[i]);
		i++;
	}
	CASE("ft_lstadd_back(&empty, n0)");
	ft_lstadd_back(&lst, n[0]);
	EXPECT(lst == n[0] && n[0]->next == NULL, "list must be n0 -> NULL");
	CASE("ft_lstadd_back(&[n0], n1)");
	ft_lstadd_back(&lst, n[1]);
	EXPECT(lst == n[0] && n[0]->next == n[1] && n[1]->next == NULL, "list must be n0 -> n1");
	CASE("ft_lstadd_back(&[n0, n1], n2)");
	ft_lstadd_back(&lst, n[2]);
	EXPECT(lst == n[0] && n[1]->next == n[2] && n[2]->next == NULL, "list must be n0 -> n1 -> n2");
	n[3]->next = n[4];
	chain = n[3];
	CASE("ft_lstadd_back(&[n0, n1, n2], [n3 -> n4])");
	ft_lstadd_back(&lst, chain);
	EXPECT(n[2]->next == n[3] && n[3]->next == n[4] && n[4]->next == NULL, "the whole chain must be appended");
	i = 0;
	while (i < 5)
		t_free(n[i++]);
}

static void	test_lstsize(void)
{
	t_list	*l;
	int		sizes[] = {1, 2, 5, 1000};
	size_t	i;

	CASE("ft_lstsize(NULL)");
	EXPECT(ft_lstsize(NULL) == 0, "expected 0, got %u", (unsigned)ft_lstsize(NULL));
	i = 0;
	while (i < sizeof(sizes) / sizeof(*sizes))
	{
		l = build(sizes[i], 0);
		CASE("ft_lstsize(<%d nodes>)", sizes[i]);
		EXPECT((long)ft_lstsize(l) == sizes[i], "expected %d, got %ld", sizes[i], (long)ft_lstsize(l));
		free_list(l, 0);
		i++;
	}
}

static void	test_lstlast(void)
{
	t_list	*l;
	t_list	*last;

	CASE("ft_lstlast(NULL)");
	EXPECT(ft_lstlast(NULL) == NULL, "expected NULL");
	l = build(1, 0);
	CASE("ft_lstlast(<1 node>)");
	EXPECT(ft_lstlast(l) == l, "expected the node itself");
	free_list(l, 0);
	l = build(7, 0);
	last = l;
	while (last->next)
		last = last->next;
	CASE("ft_lstlast(<7 nodes>)");
	EXPECT(ft_lstlast(l) == last, "expected the 7th node");
	CASE("ft_lstlast(<7 nodes>) does not modify the list");
	EXPECT(ft_lstsize(l) == 7 && last->next == NULL, "the list changed");
	free_list(l, 0);
}

static void	test_lstdelone(void)
{
	t_list	*a;
	t_list	*b;
	void	*content;

	a = build(2, 1);
	b = a->next;
	content = a->content;
	g_del_calls = 0;
	CASE("ft_lstdelone(node, del)");
	ft_lstdelone(a, del_rec);
	EXPECT(g_del_calls == 1 && g_del_seen[0] == content, "del must be called exactly once, with node->content (called %d time(s))", g_del_calls);
	EXPECT(t_is_freed(a), "the node itself was not freed");
	EXPECT(t_is_block(b) && t_is_block(b->content) && strcmp(b->content, "one") == 0,
		"the next node must NOT be freed or modified");
	free_list(b, 1);
}

static void	test_lstclear(void)
{
	t_list	*lst;
	t_list	*nodes[5];
	void	*contents[5];
	t_list	*cur;
	int		i;
	int		ok;

	lst = build(5, 1);
	cur = lst;
	i = 0;
	while (cur)
	{
		nodes[i] = cur;
		contents[i++] = cur->content;
		cur = cur->next;
	}
	g_del_calls = 0;
	CASE("ft_lstclear(&<5 nodes>, del)");
	ft_lstclear(&lst, del_rec);
	EXPECT(lst == NULL, "the list pointer must be set to NULL");
	EXPECT(g_del_calls == 5, "del must be called 5 times, called %d", g_del_calls);
	ok = 1;
	i = 0;
	while (i < 5)
	{
		ok &= t_is_freed(nodes[i]) && t_is_freed(contents[i]);
		i++;
	}
	EXPECT(ok, "not every node / content was freed");
	g_del_calls = 0;
	lst = NULL;
	CASE("ft_lstclear(&empty, del)");
	ft_lstclear(&lst, del_rec);
	EXPECT(lst == NULL && g_del_calls == 0, "del must not be called on an empty list");
	lst = build(3, 1);
	cur = lst;
	g_del_calls = 0;
	CASE("ft_lstclear(&lst->next, del) (clear from the middle)");
	ft_lstclear(&lst->next, del_rec);
	EXPECT(lst->next == NULL && g_del_calls == 2 && t_is_block(lst) && strcmp(lst->content, "zero") == 0,
		"only the nodes after the first one must be deleted, and lst->next set to NULL");
	CASE("ft_lstclear twice on the same list");
	ft_lstclear(&lst, del_rec);
	ft_lstclear(&lst, del_rec);
	EXPECT(lst == NULL, "list pointer must stay NULL");
	(void)cur;
}

static void	test_lstiter(void)
{
	t_list	*l;
	t_list	*cur;
	int		i;
	int		ok;

	l = build(5, 0);
	g_f_calls = 0;
	CASE("ft_lstiter(<5 nodes>, f)");
	ft_lstiter(l, iter_rec);
	ok = g_f_calls == 5;
	cur = l;
	i = 0;
	while (ok && cur)
	{
		ok = g_f_seen[i++] == cur->content;
		cur = cur->next;
	}
	EXPECT(ok, "f must be called on each content, in order (called %d time(s))", g_f_calls);
	g_f_calls = 0;
	CASE("ft_lstiter(NULL, f)");
	ft_lstiter(NULL, iter_rec);
	EXPECT(g_f_calls == 0, "f must not be called");
	free_list(l, 0);
}

static void	test_lstmap(void)
{
	t_list	*l;
	t_list	*m;
	t_list	*cur;
	t_list	*mc;
	int		i;
	int		ok;

	l = build(5, 1);
	g_f_calls = 0;
	g_del_calls = 0;
	CASE("ft_lstmap(<5 nodes>, upper, del)");
	m = ft_lstmap(l, map_upper, del_rec);
	if (!m)
	{
		t_fail("returned NULL");
		free_list(l, 1);
		return ;
	}
	EXPECT(g_f_calls == 5, "f must be called once per node (called %d time(s))", g_f_calls);
	EXPECT(g_del_calls == 0, "del must not be called when nothing fails (called %d time(s))", g_del_calls);
	ok = 1;
	cur = l;
	mc = m;
	i = 0;
	while (cur && mc && ok)
	{
		ok = mc != cur && t_is_block(mc) && t_is_block(mc->content) && mc->content != cur->content
			&& strcmp(cur->content, g_words[i]) == 0;
		if (ok)
		{
			ok = strlen(mc->content) == strlen(g_words[i]);
			ok = ok && toupper((unsigned char)g_words[i][0]) == ((char *)mc->content)[0];
		}
		cur = cur->next;
		mc = mc->next;
		i++;
	}
	EXPECT(ok && !cur && !mc, "the new list must hold f(content) in new nodes, same length (%d)", i);
	EXPECT(ft_lstsize(l) == 5 && strcmp(l->content, "zero") == 0, "the original list was modified");
	free_list(m, 1);
	free_list(l, 1);
	CASE("ft_lstmap(NULL, f, del)");
	EXPECT(ft_lstmap(NULL, map_upper, del_rec) == NULL, "expected NULL");
}

static void	test_lst_null(void)
{
	t_list	*lst;
	t_list	*n;

	lst = NULL;
	n = ft_lstnew("x");
	CASE("ft_lstadd_front(NULL, node)");
	ft_lstadd_front(NULL, n);
	CASE("ft_lstadd_back(NULL, node)");
	ft_lstadd_back(NULL, n);
	CASE("ft_lstadd_front(&lst, NULL)");
	ft_lstadd_front(&lst, NULL);
	CASE("ft_lstadd_back(&lst, NULL)");
	ft_lstadd_back(&lst, NULL);
	CASE("ft_lstdelone(NULL, del)");
	ft_lstdelone(NULL, del_rec);
	CASE("ft_lstclear(NULL, del)");
	ft_lstclear(NULL, del_rec);
	CASE("ft_lstiter(list, NULL)");
	ft_lstiter(n, NULL);
	CASE("ft_lstmap(list, NULL, del)");
	lst = ft_lstmap(n, NULL, del_count_only);
	if (t_is_block(lst))
		free_list(lst, 0);
	CASE("ft_lstdelone(node, NULL)");
	n->next = NULL;
	ft_lstdelone(n, NULL);
	t_free(n);
}

/* ------------------------------------------------------------------ */

void	run_part3(void)
{
	t_section("PART 3 - LINKED LIST (mandatory)");
	t_run("ft_lstnew (next must be NULL, content not copied)", test_lstnew, T_MUST, 5);
	t_run("ft_lstadd_front", test_lstadd_front, T_MUST, 5);
	t_run("ft_lstadd_back (incl. empty list, appending a chain)", test_lstadd_back, T_MUST, 5);
	t_run("ft_lstsize", test_lstsize, T_MUST, 5);
	t_run("ft_lstlast", test_lstlast, T_MUST, 5);
	t_run("ft_lstdelone (frees node + content, not next)", test_lstdelone, T_MUST, 5);
	t_run("ft_lstclear (all freed, *lst = NULL, from the middle)", test_lstclear, T_MUST, 5);
	t_run("ft_lstiter", test_lstiter, T_MUST, 5);
	t_run("ft_lstmap", test_lstmap, T_MUST, 5);
	t_run("ft_lst* with NULL arguments (UB)", test_lst_null, T_WARN, 5);
}
