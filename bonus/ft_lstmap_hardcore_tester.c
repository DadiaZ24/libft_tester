#include "../tester.h"
#include <dlfcn.h>

/*
** Uses the same malloc interposer defined in ft_hardcore_tester.c.
** set_malloc_fail_after is declared extern here.
*/
extern void	set_malloc_fail_after(int n);

static void	*dup_str(void *content)
{
	return (ft_strdup((char *)content));
}

static void	del_str(void *content)
{
	free(content);
}

static int	test_lstmap_malloc_fail_at_node()
{
	int		fails = 0;
	t_list	*lst;
	t_list	*mapped;

	printf("[HARDCORE] ft_lstmap malloc fails on first node (no crash + no leak)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	set_malloc_fail_after(-1);
	lst = ft_lstnew(ft_strdup("hello"));
	ft_lstadd_back(&lst, ft_lstnew(ft_strdup("world")));
	ft_lstadd_back(&lst, ft_lstnew(ft_strdup("foo")));

	/*
	** Allow dup of content to succeed once, then fail on ft_lstnew or next dup.
	** A protected ft_lstmap must return NULL and not leak.
	*/
	set_malloc_fail_after(1);
	mapped = ft_lstmap(lst, dup_str, del_str);
	set_malloc_fail_after(-1);

	if (mapped == NULL)
		PASS();
	else
	{
		ft_lstclear(&mapped, del_str);
		CHECK();
	}
	ft_lstclear(&lst, del_str);
	return (fails);
}

static int	test_lstmap_malloc_fail_mid()
{
	int		fails = 0;
	t_list	*lst;
	t_list	*mapped;

	printf("[HARDCORE] ft_lstmap malloc fails mid-list (no crash + no leak)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	set_malloc_fail_after(-1);
	lst = ft_lstnew(ft_strdup("a"));
	ft_lstadd_back(&lst, ft_lstnew(ft_strdup("b")));
	ft_lstadd_back(&lst, ft_lstnew(ft_strdup("c")));
	ft_lstadd_back(&lst, ft_lstnew(ft_strdup("d")));

	/* Allow array + 2 word allocs then fail — must return NULL, no leak */
	set_malloc_fail_after(3);
	mapped = ft_lstmap(lst, dup_str, del_str);
	set_malloc_fail_after(-1);

	if (mapped == NULL)
		PASS();
	else
	{
		ft_lstclear(&mapped, del_str);
		CHECK();
	}
	ft_lstclear(&lst, del_str);
	return (fails);
}

int	testing_ft_lstmap_hardcore()
{
	int	fails = 0;

	TITLE("HARDCORE: FT_LSTMAP");
	sleep(1);
	fails += test_lstmap_malloc_fail_at_node();
	sleep(1);
	fails += test_lstmap_malloc_fail_mid();
	sleep(1);

	printf("\n===== Hardcore lstmap Testing Complete =====\n");
	return (fails);
}
