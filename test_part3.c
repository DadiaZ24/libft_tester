#include "tester.h"

/*
** PART 3 - Linked list (mandatory in subject v19.3, no longer a bonus)
** Lists are built by the tester itself (mk / mklist), never with your
** ft_lstnew / ft_lstadd_*: every function is tested on its own.
** Freed nodes are filled with 0xDF: reading node->next after free(node)
** gives 0xDFDFDFDFDFDFDFDF and crashes immediately.
*/

#define DEEP 1000000

static int	g_del_calls;
static void	*g_del_seen[16];
static int	g_f_calls;
static void	*g_f_seen[16];

static const char	*g_words[] = {"zero", "one", "two", "three", "four", "five", "six", "seven"};

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

/* a node allocated by the tester, next = NULL */
static t_list	*mk(void *content)
{
	t_list	*n;

	n = malloc(sizeof(t_list));
	if (!n)
		return (NULL);
	n->content = content;
	n->next = NULL;
	return (n);
}

/* n nodes holding g_words (malloc'd copies if alloc) */
static t_list	*mklist(int n, int alloc)
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
		node = mk(alloc ? (void *)dupstr(g_words[i % 8]) : (void *)g_words[i % 8]);
		if (last)
			last->next = node;
		else
			head = node;
		last = node;
		i++;
	}
	return (head);
}

/* stops at a node that is not (or no longer) a live block */
static void	free_list(t_list *l, int free_content)
{
	t_list	*next;

	while (l && t_is_block(l))
	{
		next = l->next;
		if (free_content)
			t_free(l->content);
		t_free(l);
		l = next;
	}
}

static int	count(t_list *l)
{
	int	n;

	n = 0;
	while (l && n <= DEEP + 1)
	{
		l = l->next;
		n++;
	}
	return (n);
}

/* the nodes of l, in order, match nodes[] (n of them) */
static int	same_nodes(t_list *l, t_list **nodes, int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		if (l != nodes[i])
			return (0);
		l = l->next;
		i++;
	}
	return (l == NULL);
}

static void	del_rec(void *p)
{
	if (g_del_calls < 16)
		g_del_seen[g_del_calls] = p;
	g_del_calls++;
	free(p);
}

static void	del_count(void *p)
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

static void	iter_upper(void *p)
{
	char	*s;

	g_f_calls++;
	s = p;
	while (*s)
	{
		*s = (char)toupper((unsigned char)*s);
		s++;
	}
}

static void	nothing(void *p)
{
	(void)p;
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
		r[i] = (char)toupper((unsigned char)r[i]);
		i++;
	}
	return (r);
}

static void	*map_same(void *p)
{
	g_f_calls++;
	return (p);
}

/* ================================================================== */
/* ft_lstnew                                                          */
/* ================================================================== */

static void	lstnew_content(void)
{
	t_list	*n;
	int		x;

	CASE("ft_lstnew(\"hello\")");
	n = ft_lstnew("hello");
	EXPECT(n && n->content == (void *)"hello", "content must be the given pointer (not a copy)");
	t_free(n);
	CASE("ft_lstnew(&x)");
	n = ft_lstnew(&x);
	EXPECT(n && n->content == &x, "content must be the given pointer");
	t_free(n);
}

static void	lstnew_next(void)
{
	t_list	*n;

	CASE("ft_lstnew(\"hello\")->next");
	n = ft_lstnew("hello");
	EXPECT(n && n->next == NULL, "next must be NULL (the tester's malloc returns garbage on purpose)");
	t_free(n);
}

static void	lstnew_null(void)
{
	t_list	*n;

	CASE("ft_lstnew(NULL)");
	n = ft_lstnew(NULL);
	EXPECT(n != NULL, "a node holding NULL is a valid node, got NULL");
	EXPECT(!n || (n->content == NULL && n->next == NULL), "content and next must be NULL");
	t_free(n);
}

static void	lstnew_block(void)
{
	t_list	*n;

	CASE("ft_lstnew(\"x\") is a malloc'd node");
	n = ft_lstnew("x");
	if (!n)
	{
		t_fail("returned NULL");
		return ;
	}
	EXPECT(t_is_block(n), "the node is not the start of a malloc'd block");
	EXPECT(t_block_size(n) >= sizeof(t_list), "allocated %zu bytes, a t_list needs %zu",
		t_block_size(n), sizeof(t_list));
	t_free(n);
}

static void	lstnew_many(void)
{
	t_list	*n[1000];
	int		i;
	int		ok;

	CASE("1000 x ft_lstnew(&n[i])");
	ok = 1;
	i = 0;
	while (i < 1000)
	{
		n[i] = ft_lstnew(&n[i]);
		ok &= n[i] && n[i]->content == &n[i] && !n[i]->next && (i == 0 || n[i] != n[i - 1]);
		i++;
	}
	EXPECT(ok, "every node must be new, hold its own content and have next = NULL");
	i = 0;
	while (i < 1000)
		t_free(n[i++]);
}

static void	mf_lstnew(void)
{
	t_list	*n;

	CASE("ft_lstnew(\"x\"), its malloc fails");
	t_arm();
	n = ft_lstnew("x");
	t_disarm();
	if (t_injected())
		EXPECT(n == NULL, "returned non-NULL although its malloc returned NULL");
	t_free(n);
}

static const t_test	g_ft_lstnew[] = {
	TEST("content is the pointer given", "The node stores the pointer itself, it does not copy what it \
points to.", lstnew_content),
	TEST("next is NULL", "The tester's malloc fills blocks with garbage: forgetting next = NULL leaves a \
garbage pointer.", lstnew_next),
	TEST("NULL content", "ft_lstnew(NULL) is a valid node holding NULL.", lstnew_null),
	TEST("a malloc'd t_list", "The node is a malloc'd block of at least sizeof(t_list).", lstnew_block),
	TEST("1000 independent nodes", "Every call returns a new node (no static node).", lstnew_many),
	TEST_MF("malloc fails", "When its malloc fails, lstnew returns NULL.", mf_lstnew),
};

/* ================================================================== */
/* ft_lstadd_front                                                    */
/* ================================================================== */

static void	front_empty(void)
{
	t_list	*lst;
	t_list	*a;

	lst = NULL;
	a = mk("A");
	a->next = (t_list *)0xDEAD;
	CASE("ft_lstadd_front(&empty, A) with A->next = garbage");
	ft_lstadd_front(&lst, a);
	EXPECT(lst == a, "*lst must be A");
	EXPECT(a->next == NULL, "A->next must become the old head (NULL)");
	t_free(a);
}

static void	front_two(void)
{
	t_list	*lst;
	t_list	*a;
	t_list	*b;

	a = mk("A");
	b = mk("B");
	lst = a;
	CASE("ft_lstadd_front(&[A], B)");
	ft_lstadd_front(&lst, b);
	EXPECT(lst == b && b->next == a && a->next == NULL, "list must be B -> A");
	free_list(lst, 0);
}

static void	front_many(void)
{
	t_list	*lst;
	t_list	*n[5];
	t_list	*exp[5];
	int		i;

	lst = NULL;
	i = 0;
	while (i < 5)
	{
		n[i] = mk((void *)g_words[i]);
		CASE("ft_lstadd_front(&lst, n%d)", i);
		ft_lstadd_front(&lst, n[i]);
		i++;
	}
	i = 0;
	while (i < 5)
	{
		exp[i] = n[4 - i];
		i++;
	}
	CASE("5 x ft_lstadd_front");
	EXPECT(same_nodes(lst, exp, 5), "list must be n4 -> n3 -> n2 -> n1 -> n0");
	free_list(lst, 0);
}

static void	front_untouched(void)
{
	t_list	*lst;
	t_list	*n;

	lst = mklist(3, 1);
	n = mk(dupstr("new"));
	CASE("ft_lstadd_front(&[zero, one, two], new)");
	ft_lstadd_front(&lst, n);
	EXPECT(lst == n && count(lst) == 4, "list must be new -> zero -> one -> two");
	EXPECT(!strcmp(lst->content, "new") && !strcmp(lst->next->content, "zero")
		&& !strcmp(lst->next->next->next->content, "two"), "the contents changed");
	free_list(lst, 1);
}

static void	front_many_fast(void)
{
	t_list	*lst;
	int		i;

	lst = NULL;
	CASE("100 000 x ft_lstadd_front");
	i = 0;
	while (i < 100000)
	{
		ft_lstadd_front(&lst, mk(NULL));
		i++;
	}
	EXPECT(count(lst) == 100000, "wrong number of nodes");
	free_list(lst, 0);
}

static void	front_null(void)
{
	t_list	*lst;
	t_list	*n;

	lst = NULL;
	n = mk("x");
	CASE("ft_lstadd_front(NULL, node)");
	ft_lstadd_front(NULL, n);
	CASE("ft_lstadd_front(&lst, NULL)");
	ft_lstadd_front(&lst, NULL);
	t_free(n);
}

static const t_test	g_ft_lstadd_front[] = {
	TEST("empty list", "*lst becomes new and new->next the old head (NULL), even if new->next held \
garbage.", front_empty),
	TEST("two nodes", "new goes before the old head: B -> A.", front_two),
	TEST("5 nodes, order", "Each new node becomes the head.", front_many),
	TEST("the other nodes are not touched", "Only new->next and *lst change.", front_untouched),
	TEST("100 000 nodes", "Adding at the front is O(1): no walk through the list.", front_many_fast),
	TEST_UB("NULL lst / new", "Not defined by the subject, but evaluators try it: no crash.", front_null),
};

/* ================================================================== */
/* ft_lstsize / ft_lstlast                                            */
/* ================================================================== */

static void	size_check(int n)
{
	t_list			*l;
	unsigned int	got;

	l = mklist(n, 0);
	CASE("ft_lstsize(<%d nodes>)", n);
	got = ft_lstsize(l);
	EXPECT(got == (unsigned int)n, "expected %d, got %u", n, got);
	free_list(l, 0);
}

static void	size_null(void)
{
	CASE("ft_lstsize(NULL)");
	EXPECT(ft_lstsize(NULL) == 0, "expected 0, got %u", ft_lstsize(NULL));
}

static void	size_one(void)
{
	size_check(1);
}

static void	size_five(void)
{
	size_check(2);
	size_check(5);
}

static void	size_thousand(void)
{
	size_check(1000);
}

static void	size_untouched(void)
{
	t_list	*l;
	t_list	*nodes[6];
	t_list	*cur;
	int		i;

	l = mklist(6, 0);
	cur = l;
	i = 0;
	while (cur)
	{
		nodes[i++] = cur;
		cur = cur->next;
	}
	CASE("ft_lstsize(<6 nodes>) twice");
	ft_lstsize(l);
	EXPECT(ft_lstsize(l) == 6 && same_nodes(l, nodes, 6), "the list changed");
	free_list(l, 0);
}

static void	size_deep(void)
{
	t_list	*l;

	l = mklist(DEEP, 0);
	CASE("ft_lstsize(<1 000 000 nodes>)");
	EXPECT(ft_lstsize(l) == DEEP, "wrong size");
	free_list(l, 0);
}

static const t_test	g_ft_lstsize[] = {
	TEST("NULL list: 0", "An empty list has 0 nodes.", size_null),
	TEST("1 node", "A single node: 1.", size_one),
	TEST("2 and 5 nodes", "Every node counts once.", size_five),
	TEST("1000 nodes", "A longer list.", size_thousand),
	TEST("the list is not modified", "Counting only reads the list.", size_untouched),
	TEST_UB_SLOW("1 000 000 nodes", "A recursive lstsize overflows the stack on a very long list.", size_deep),
};

static void	last_null(void)
{
	CASE("ft_lstlast(NULL)");
	EXPECT(ft_lstlast(NULL) == NULL, "expected NULL");
}

static void	last_one(void)
{
	t_list	*l;

	l = mklist(1, 0);
	CASE("ft_lstlast(<1 node>)");
	EXPECT(ft_lstlast(l) == l, "expected the node itself");
	free_list(l, 0);
}

static void	last_two(void)
{
	t_list	*l;

	l = mklist(2, 0);
	CASE("ft_lstlast(<2 nodes>)");
	EXPECT(ft_lstlast(l) == l->next, "expected the 2nd node");
	free_list(l, 0);
}

static void	last_seven(void)
{
	t_list	*l;
	t_list	*last;

	l = mklist(7, 0);
	last = l;
	while (last->next)
		last = last->next;
	CASE("ft_lstlast(<7 nodes>)");
	EXPECT(ft_lstlast(l) == last, "expected the 7th node");
	free_list(l, 0);
}

static void	last_untouched(void)
{
	t_list	*l;
	t_list	*nodes[4];
	t_list	*cur;
	int		i;

	l = mklist(4, 0);
	cur = l;
	i = 0;
	while (cur)
	{
		nodes[i++] = cur;
		cur = cur->next;
	}
	CASE("ft_lstlast(<4 nodes>) twice");
	ft_lstlast(l);
	EXPECT(ft_lstlast(l) == nodes[3] && same_nodes(l, nodes, 4), "the list changed");
	free_list(l, 0);
}

static void	last_deep(void)
{
	t_list	*l;
	t_list	*last;

	l = mklist(DEEP, 0);
	last = l;
	while (last->next)
		last = last->next;
	CASE("ft_lstlast(<1 000 000 nodes>)");
	EXPECT(ft_lstlast(l) == last, "wrong last node");
	free_list(l, 0);
}

static const t_test	g_ft_lstlast[] = {
	TEST("NULL list: NULL", "An empty list has no last node.", last_null),
	TEST("1 node: itself", "The only node is the last one.", last_one),
	TEST("2 nodes", "The second node (whose next is NULL).", last_two),
	TEST("7 nodes", "The node whose next is NULL, not the one before.", last_seven),
	TEST("the list is not modified", "lstlast only reads the list.", last_untouched),
	TEST_UB_SLOW("1 000 000 nodes", "A recursive lstlast overflows the stack on a very long list.", last_deep),
};

/* ================================================================== */
/* ft_lstadd_back                                                     */
/* ================================================================== */

static void	back_empty(void)
{
	t_list	*lst;
	t_list	*a;

	lst = NULL;
	a = mk("A");
	CASE("ft_lstadd_back(&empty, A)");
	ft_lstadd_back(&lst, a);
	EXPECT(lst == a && a->next == NULL, "*lst must become A");
	t_free(a);
}

static void	back_one(void)
{
	t_list	*lst;
	t_list	*b;

	lst = mk("A");
	b = mk("B");
	CASE("ft_lstadd_back(&[A], B)");
	ft_lstadd_back(&lst, b);
	EXPECT(lst->next == b && b->next == NULL, "list must be A -> B");
	free_list(lst, 0);
}

static void	back_many(void)
{
	t_list	*lst;
	t_list	*n[5];
	int		i;

	lst = NULL;
	i = 0;
	while (i < 5)
	{
		n[i] = mk((void *)g_words[i]);
		CASE("ft_lstadd_back(&lst, n%d)", i);
		ft_lstadd_back(&lst, n[i]);
		i++;
	}
	CASE("5 x ft_lstadd_back");
	EXPECT(same_nodes(lst, n, 5), "list must be n0 -> n1 -> n2 -> n3 -> n4");
	free_list(lst, 0);
}

static void	back_chain(void)
{
	t_list	*lst;
	t_list	*n[5];
	int		i;

	i = 0;
	while (i < 5)
	{
		n[i] = mk((void *)g_words[i]);
		i++;
	}
	n[0]->next = n[1];
	n[1]->next = n[2];
	n[3]->next = n[4];
	lst = n[0];
	CASE("ft_lstadd_back(&[n0 -> n1 -> n2], [n3 -> n4])");
	ft_lstadd_back(&lst, n[3]);
	EXPECT(same_nodes(lst, n, 5), "the whole chain must be appended: n0 -> n1 -> n2 -> n3 -> n4");
	free_list(lst, 0);
}

static void	back_untouched(void)
{
	t_list	*lst;

	lst = mklist(3, 1);
	CASE("ft_lstadd_back(&[zero, one, two], new)");
	ft_lstadd_back(&lst, mk(dupstr("new")));
	EXPECT(count(lst) == 4 && !strcmp(lst->content, "zero") && !strcmp(lst->next->next->content, "two")
		&& !strcmp(lst->next->next->next->content, "new"), "list must be zero -> one -> two -> new");
	free_list(lst, 1);
}

static void	back_many_slow(void)
{
	t_list	*lst;
	int		i;

	lst = NULL;
	CASE("20 000 x ft_lstadd_back");
	i = 0;
	while (i < 20000)
	{
		ft_lstadd_back(&lst, mk(NULL));
		i++;
	}
	EXPECT(count(lst) == 20000, "wrong size");
	free_list(lst, 0);
}

static void	back_null(void)
{
	t_list	*lst;
	t_list	*n;

	lst = NULL;
	n = mk("x");
	CASE("ft_lstadd_back(NULL, node)");
	ft_lstadd_back(NULL, n);
	CASE("ft_lstadd_back(&lst, NULL)");
	ft_lstadd_back(&lst, NULL);
	CASE("ft_lstadd_back(&[node], NULL)");
	lst = n;
	ft_lstadd_back(&lst, NULL);
	t_free(n);
}

static const t_test	g_ft_lstadd_back[] = {
	TEST("empty list: *lst = new", "Adding to an empty list must set *lst itself (lst is a t_list **).", back_empty),
	TEST("one node", "new goes after the last node: A -> B.", back_one),
	TEST("5 nodes, order", "Each node goes at the end, in order.", back_many),
	TEST("appending a whole chain", "new can be the start of a chain: everything after it stays attached.", back_chain),
	TEST("the other nodes are not touched", "Only the old last node's next changes.", back_untouched),
	TEST_SLOW("20 000 nodes", "20 000 appends walk the list each time: a slow walk times out.", back_many_slow),
	TEST_UB("NULL lst / new", "Not defined by the subject, but evaluators try it: no crash.", back_null),
};

/* ================================================================== */
/* ft_lstdelone / ft_lstclear                                         */
/* ================================================================== */

static void	delone_del(void)
{
	t_list	*a;
	void	*content;

	a = mklist(2, 1);
	content = a->content;
	free_list(a->next, 1);
	a->next = NULL;
	g_del_calls = 0;
	CASE("ft_lstdelone(node, del)");
	ft_lstdelone(a, del_rec);
	EXPECT(g_del_calls == 1 && g_del_seen[0] == content,
		"del must be called exactly once, with node->content (called %d time(s))", g_del_calls);
}

static void	delone_freed(void)
{
	t_list	*a;

	a = mklist(1, 1);
	CASE("ft_lstdelone(node, del): the node");
	ft_lstdelone(a, del_rec);
	EXPECT(t_is_freed(a), "the node itself was not freed");
}

static void	delone_next(void)
{
	t_list	*a;
	t_list	*b;

	a = mklist(3, 1);
	b = a->next;
	CASE("ft_lstdelone(first of 3 nodes, del): the next nodes");
	ft_lstdelone(a, del_rec);
	EXPECT(t_is_block(b) && t_is_block(b->content) && !strcmp(b->content, "one")
		&& t_is_block(b->next) && !strcmp(b->next->content, "two"),
		"the next nodes must NOT be freed or modified");
	free_list(b, 1);
}

static void	delone_count(void)
{
	t_list	*a;
	t_list	*b;

	a = mklist(4, 1);
	b = a->next;
	g_del_calls = 0;
	CASE("ft_lstdelone(first of 4 nodes, del)");
	ft_lstdelone(a, del_rec);
	EXPECT(g_del_calls == 1, "del called %d time(s): only the node's own content", g_del_calls);
	free_list(b, 1);
}

static void	delone_static(void)
{
	t_list	*a;

	a = mk("static string");
	g_del_calls = 0;
	CASE("ft_lstdelone(node, del that frees nothing)");
	ft_lstdelone(a, del_count);
	EXPECT(g_del_calls == 1 && t_is_freed(a), "del must be called and the node freed");
}

static void	delone_null(void)
{
	t_list	*n;

	CASE("ft_lstdelone(NULL, del)");
	ft_lstdelone(NULL, del_rec);
	CASE("ft_lstdelone(node, NULL)");
	n = mk("x");
	ft_lstdelone(n, NULL);
	t_free(n);
}

static const t_test	g_ft_lstdelone[] = {
	TEST("del called once with content", "del(node->content) exactly once: the content is freed by del, \
not by free().", delone_del),
	TEST("the node is freed", "After del, the node itself is freed.", delone_freed),
	TEST("the next nodes stay", "lstdelone deletes ONE node: next is neither freed nor followed.", delone_next),
	TEST("del not called on the others", "Only the given node's content goes to del.", delone_count),
	TEST("content that del doesn't free", "Whatever del does, it is called and the node is freed.", delone_static),
	TEST_UB("NULL node / del", "Not defined by the subject, but evaluators try it: no crash.", delone_null),
};

static void	clear_all(void)
{
	t_list	*lst;
	t_list	*nodes[5];
	void	*contents[5];
	t_list	*cur;
	int		i;
	int		ok;

	lst = mklist(5, 1);
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
	EXPECT(g_del_calls == 5, "del must be called 5 times, called %d", g_del_calls);
	ok = 1;
	i = 0;
	while (i < 5)
	{
		ok &= t_is_freed(nodes[i]) && t_is_freed(contents[i]);
		i++;
	}
	EXPECT(ok, "not every node / content was freed");
}

static void	clear_null_ptr(void)
{
	t_list	*lst;

	lst = mklist(3, 1);
	CASE("ft_lstclear(&<3 nodes>, del): *lst");
	ft_lstclear(&lst, del_rec);
	EXPECT(lst == NULL, "the list pointer must be set to NULL");
}

static void	clear_empty(void)
{
	t_list	*lst;

	lst = NULL;
	g_del_calls = 0;
	CASE("ft_lstclear(&empty, del)");
	ft_lstclear(&lst, del_rec);
	EXPECT(lst == NULL && g_del_calls == 0, "del must not be called on an empty list");
}

static void	clear_middle(void)
{
	t_list	*lst;

	lst = mklist(3, 1);
	g_del_calls = 0;
	CASE("ft_lstclear(&lst->next, del) (clear from the middle)");
	ft_lstclear(&lst->next, del_rec);
	EXPECT(lst->next == NULL && g_del_calls == 2 && t_is_block(lst) && !strcmp(lst->content, "zero"),
		"only the nodes after the first one must be deleted, and lst->next set to NULL");
	free_list(lst, 1);
}

static void	clear_twice(void)
{
	t_list	*lst;

	lst = mklist(3, 1);
	CASE("ft_lstclear twice on the same list");
	ft_lstclear(&lst, del_rec);
	ft_lstclear(&lst, del_rec);
	EXPECT(lst == NULL, "list pointer must stay NULL");
}

static void	clear_each(void)
{
	t_list	*lst;
	void	*contents[8];
	t_list	*cur;
	int		i;
	int		j;
	int		ok;

	lst = mklist(8, 1);
	cur = lst;
	i = 0;
	while (cur)
	{
		contents[i++] = cur->content;
		cur = cur->next;
	}
	g_del_calls = 0;
	CASE("ft_lstclear(&<8 nodes>, del): del's arguments");
	ft_lstclear(&lst, del_rec);
	ok = g_del_calls == 8;
	i = 0;
	while (ok && i < 8)
	{
		j = 0;
		while (j < 8 && g_del_seen[j] != contents[i])
			j++;
		ok = j < 8;
		i++;
	}
	EXPECT(ok, "del must get each content exactly once (called %d time(s))", g_del_calls);
}

static void	clear_deep(void)
{
	t_list	*l;

	l = mklist(DEEP, 0);
	g_del_calls = 0;
	CASE("ft_lstclear(<1 000 000 nodes>, del)");
	ft_lstclear(&l, del_count);
	EXPECT(l == NULL && g_del_calls == DEEP, "del called %d time(s)", g_del_calls);
}

static void	clear_null(void)
{
	t_list	*lst;

	CASE("ft_lstclear(NULL, del)");
	ft_lstclear(NULL, del_rec);
	lst = mklist(2, 0);
	CASE("ft_lstclear(&lst, NULL)");
	ft_lstclear(&lst, NULL);
	if (lst)
		free_list(lst, 0);
}

static const t_test	g_ft_lstclear[] = {
	TEST("every node and content freed", "del on every content and free() on every node: nothing leaks.", clear_all),
	TEST("*lst set to NULL", "After clearing, the caller's pointer is NULL (not a dangling pointer).", clear_null_ptr),
	TEST("empty list", "Clearing an empty list does nothing and never calls del.", clear_empty),
	TEST("from the middle", "lstclear(&lst->next, del) deletes the rest and sets lst->next to NULL.", clear_middle),
	TEST("twice", "The second clear gets an empty list: no double free.", clear_twice),
	TEST("del gets each content once", "Each content goes to del exactly once (reading node->next after \
freeing the node crashes here: freed memory is poisoned).", clear_each),
	TEST_UB_SLOW("1 000 000 nodes", "A recursive lstclear overflows the stack on a very long list.", clear_deep),
	TEST_UB("NULL lst / del", "Not defined by the subject, but evaluators try it: no crash.", clear_null),
};

/* ================================================================== */
/* ft_lstiter / ft_lstmap                                             */
/* ================================================================== */

static void	iter_order(void)
{
	t_list	*l;
	t_list	*cur;
	int		i;
	int		ok;

	l = mklist(5, 0);
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
	free_list(l, 0);
}

static void	iter_empty(void)
{
	g_f_calls = 0;
	CASE("ft_lstiter(NULL, f)");
	ft_lstiter(NULL, iter_rec);
	EXPECT(g_f_calls == 0, "f must not be called");
}

static void	iter_structure(void)
{
	t_list	*l;
	t_list	*nodes[4];
	t_list	*cur;
	int		i;

	l = mklist(4, 0);
	cur = l;
	i = 0;
	while (cur)
	{
		nodes[i++] = cur;
		cur = cur->next;
	}
	CASE("ft_lstiter(<4 nodes>, f): the list");
	ft_lstiter(l, iter_rec);
	EXPECT(same_nodes(l, nodes, 4), "the nodes changed");
	free_list(l, 0);
}

static void	iter_modify(void)
{
	t_list	*l;

	l = mklist(3, 1);
	g_f_calls = 0;
	CASE("ft_lstiter(<zero, one, two>, toupper)");
	ft_lstiter(l, iter_upper);
	EXPECT(!strcmp(l->content, "ZERO") && !strcmp(l->next->content, "ONE")
		&& !strcmp(l->next->next->content, "TWO"), "f must modify each content in place");
	free_list(l, 1);
}

static void	iter_one(void)
{
	t_list	*l;

	l = mklist(1, 0);
	g_f_calls = 0;
	CASE("ft_lstiter(<1 node>, f)");
	ft_lstiter(l, iter_rec);
	EXPECT(g_f_calls == 1 && g_f_seen[0] == l->content, "f must be called once with the content");
	free_list(l, 0);
}

static void	iter_deep(void)
{
	t_list	*l;

	l = mklist(DEEP, 0);
	CASE("ft_lstiter(<1 000 000 nodes>, f)");
	ft_lstiter(l, nothing);
	free_list(l, 0);
}

static void	iter_null(void)
{
	t_list	*l;

	l = mklist(2, 0);
	CASE("ft_lstiter(list, NULL)");
	ft_lstiter(l, NULL);
	free_list(l, 0);
}

static const t_test	g_ft_lstiter[] = {
	TEST("f on every content, in order", "f(content) for each node, from the head to the end.", iter_order),
	TEST("empty list", "Nothing to iterate: f is never called.", iter_empty),
	TEST("the nodes are not touched", "lstiter gives the contents to f, it does not change the list.", iter_structure),
	TEST("f can modify the contents", "f gets the content pointer itself, not a copy.", iter_modify),
	TEST("1 node", "f is called exactly once.", iter_one),
	TEST_UB_SLOW("1 000 000 nodes", "A recursive lstiter overflows the stack on a very long list.", iter_deep),
	TEST_UB("NULL f", "Not defined by the subject, but evaluators try it: no crash.", iter_null),
};

static void	map_result(void)
{
	t_list	*l;
	t_list	*m;
	t_list	*mc;
	int		i;
	int		ok;

	l = mklist(5, 1);
	g_f_calls = 0;
	CASE("ft_lstmap(<5 nodes>, upper, del)");
	m = ft_lstmap(l, map_upper, del_rec);
	ok = m != NULL;
	mc = m;
	i = 0;
	while (ok && i < 5)
	{
		ok = mc && t_is_block(mc->content) && strlen(mc->content) == strlen(g_words[i])
			&& ((char *)mc->content)[0] == toupper((unsigned char)g_words[i][0]);
		mc = mc ? mc->next : NULL;
		i++;
	}
	EXPECT(ok && !mc, "the new list must hold f(content) for each node, in order, same length");
	EXPECT(g_f_calls == 5, "f must be called once per node (called %d time(s))", g_f_calls);
	free_list(m, 1);
	free_list(l, 1);
}

static void	map_new_nodes(void)
{
	t_list	*l;
	t_list	*m;
	t_list	*cur;
	t_list	*mc;
	int		ok;

	l = mklist(4, 1);
	CASE("ft_lstmap(<4 nodes>, upper, del): the nodes");
	m = ft_lstmap(l, map_upper, del_rec);
	ok = m != NULL;
	cur = l;
	mc = m;
	while (ok && cur && mc)
	{
		ok = mc != cur && t_is_block(mc) && t_block_size(mc) >= sizeof(t_list);
		cur = cur->next;
		mc = mc->next;
	}
	EXPECT(ok, "every node of the new list must be a new malloc'd node");
	free_list(m, 1);
	free_list(l, 1);
}

static void	map_orig(void)
{
	t_list	*l;
	t_list	*m;
	t_list	*nodes[5];
	t_list	*cur;
	int		i;
	int		ok;

	l = mklist(5, 1);
	cur = l;
	i = 0;
	while (cur)
	{
		nodes[i++] = cur;
		cur = cur->next;
	}
	CASE("ft_lstmap(<5 nodes>, upper, del): the original list");
	m = ft_lstmap(l, map_upper, del_rec);
	ok = same_nodes(l, nodes, 5);
	cur = l;
	i = 0;
	while (ok && cur)
	{
		ok = t_is_block(cur->content) && !strcmp(cur->content, g_words[i++]);
		cur = cur->next;
	}
	EXPECT(ok, "the original list (nodes or contents) was modified or freed");
	free_list(m, 1);
	free_list(l, 1);
}

static void	map_nodel(void)
{
	t_list	*l;
	t_list	*m;

	l = mklist(5, 1);
	g_del_calls = 0;
	CASE("ft_lstmap(<5 nodes>, upper, del): del");
	m = ft_lstmap(l, map_upper, del_rec);
	EXPECT(g_del_calls == 0, "del must not be called when nothing fails (called %d time(s))", g_del_calls);
	free_list(m, 1);
	free_list(l, 1);
}

static void	map_empty(void)
{
	g_f_calls = 0;
	CASE("ft_lstmap(NULL, f, del)");
	EXPECT(ft_lstmap(NULL, map_upper, del_rec) == NULL, "expected NULL");
	EXPECT(g_f_calls == 0, "f must not be called");
}

static void	map_one(void)
{
	t_list	*l;
	t_list	*m;

	l = mklist(1, 1);
	CASE("ft_lstmap(<1 node>, upper, del)");
	m = ft_lstmap(l, map_upper, del_rec);
	EXPECT(m && m != l && m->next == NULL && m->content && !strcmp(m->content, "ZERO"),
		"expected a new single node holding \"ZERO\", with next = NULL");
	free_list(m, 1);
	free_list(l, 1);
}

static void	map_scenario(int len)
{
	t_list	*l;
	t_list	*m;
	t_list	*cur;
	int		i;
	int		ok;

	l = mklist(len, 1);
	g_del_calls = 0;
	g_f_calls = 0;
	CASE("ft_lstmap(<%d nodes>, strdup, del), a malloc fails", len);
	t_arm();
	m = ft_lstmap(l, map_upper, del_rec);
	t_disarm();
	if (t_injected())
		EXPECT(m == NULL, "returned non-NULL although one of its mallocs returned NULL");
	else
		EXPECT(m != NULL && count(m) == len, "wrong result");
	ok = 1;
	cur = l;
	i = 0;
	while (cur && ok)
	{
		ok = t_is_block(cur) && t_is_block(cur->content) && !strcmp(cur->content, g_words[i++]);
		cur = cur->next;
	}
	EXPECT(ok && i == len, "the ORIGINAL list was freed or modified");
	while (m && t_is_block(m))
	{
		cur = m->next;
		t_free(m->content);
		free(m);
		m = cur;
	}
	free_list(l, 1);
}

static void	mf_map1(void)
{
	map_scenario(1);
}

static void	mf_map2(void)
{
	map_scenario(2);
}

static void	mf_map6(void)
{
	map_scenario(6);
}

static void	map_deep(void)
{
	t_list	*l;
	t_list	*m;

	l = mklist(DEEP, 0);
	g_f_calls = 0;
	CASE("ft_lstmap(<1 000 000 nodes>, f, del)");
	m = ft_lstmap(l, map_same, del_count);
	EXPECT(m && g_f_calls == DEEP, "f called %d time(s)", g_f_calls);
	free_list(m, 0);
	free_list(l, 0);
}

static void	map_null(void)
{
	t_list	*l;
	t_list	*m;

	l = mklist(2, 0);
	CASE("ft_lstmap(list, NULL, del)");
	m = ft_lstmap(l, NULL, del_count);
	if (t_is_block(m))
		free_list(m, 0);
	CASE("ft_lstmap(list, f, NULL)");
	m = ft_lstmap(l, map_same, NULL);
	if (t_is_block(m))
		free_list(m, 0);
	free_list(l, 0);
}

static const t_test	g_ft_lstmap[] = {
	TEST("new list of f(content)", "A new list holding f(content) of each node, in the same order and \
length.", map_result),
	TEST("new nodes", "Every node of the result is newly malloc'd (the old nodes are not reused).", map_new_nodes),
	TEST("the original list is untouched", "Mapping reads the list: its nodes and contents stay as they were.", map_orig),
	TEST("del not called when all goes well", "del is only for cleaning up after a failed malloc.", map_nodel),
	TEST("NULL list: NULL", "Mapping an empty list gives NULL, and f is never called.", map_empty),
	TEST("1 node", "A single new node, whose next is NULL.", map_one),
	TEST_MF("malloc fails, 1 node", "When a node can't be allocated: del(f's result for it), clear the \
new list, return NULL. The original list stays.", mf_map1),
	TEST_MF("malloc fails, 2 nodes", "Same with a failure on the 1st or the 2nd node.", mf_map2),
	TEST_MF("malloc fails, 6 nodes", "Same with a failure on any of 6 nodes: every node already made \
must be freed (with del on its content).", mf_map6),
	TEST_UB_SLOW("1 000 000 nodes", "A recursive lstmap overflows the stack on a very long list.", map_deep),
	TEST_UB("NULL f / del", "Not defined by the subject, but evaluators try it: no crash.", map_null),
};

/* ================================================================== */

void	run_part3(void)
{
	t_section("PART 3 - LINKED LIST (mandatory)");
	GROUP("ft_lstnew", g_ft_lstnew);
	GROUP("ft_lstadd_front", g_ft_lstadd_front);
	GROUP("ft_lstsize", g_ft_lstsize);
	GROUP("ft_lstlast", g_ft_lstlast);
	GROUP("ft_lstadd_back", g_ft_lstadd_back);
	GROUP("ft_lstdelone", g_ft_lstdelone);
	GROUP("ft_lstclear", g_ft_lstclear);
	GROUP("ft_lstiter", g_ft_lstiter);
	GROUP("ft_lstmap", g_ft_lstmap);
}
