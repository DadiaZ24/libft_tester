#include "../tester.h"

// ===== FT_LSTITER =====

static int g_iter_count = 0;

static void count_iter(void *content)
{
	(void)content;
	g_iter_count++;
}

static int test_lstiter_count()
{
	int fails = 0;
	printf("[TEST] lstiter Counts All Nodes\n");

	t_list *lst = ft_lstnew("a");
	ft_lstadd_back(&lst, ft_lstnew("b"));
	ft_lstadd_back(&lst, ft_lstnew("c"));

	g_iter_count = 0;
	ft_lstiter(lst, count_iter);

	if (g_iter_count == 3)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	ft_lstclear(&lst, NULL);
	return fails;
}

static int test_lstiter_empty()
{
	int fails = 0;
	printf("[TEST] lstiter Empty List (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);

	g_iter_count = 0;
	ft_lstiter(NULL, count_iter);
	if (g_iter_count == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static void upcase_content(void *content)
{
	char *s = (char *)content;
	while (*s)
	{
		if (*s >= 'a' && *s <= 'z')
			*s -= 32;
		s++;
	}
}

static int test_lstiter_modifies()
{
	int fails = 0;
	printf("[TEST] lstiter Modifies Content\n");

	char buf1[] = "hello";
	char buf2[] = "world";
	t_list *lst = ft_lstnew(buf1);
	ft_lstadd_back(&lst, ft_lstnew(buf2));

	ft_lstiter(lst, upcase_content);

	if (strcmp(lst->content, "HELLO") == 0
		&& strcmp(lst->next->content, "WORLD") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	ft_lstclear(&lst, NULL);
	return fails;
}

// ===== FT_LSTMAP =====

static void *dup_content(void *content)
{
	return ft_strdup((char *)content);
}

static void del_free(void *content)
{
	free(content);
}

static int test_lstmap_basic()
{
	int fails = 0;
	printf("[TEST] lstmap Basic\n");

	t_list *lst = ft_lstnew(ft_strdup("hello"));
	ft_lstadd_back(&lst, ft_lstnew(ft_strdup("world")));

	t_list *mapped = ft_lstmap(lst, dup_content, del_free);

	if (mapped && strcmp(mapped->content, "hello") == 0
		&& mapped->next && strcmp(mapped->next->content, "world") == 0
		&& mapped->next->next == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	ft_lstclear(&lst, del_free);
	ft_lstclear(&mapped, del_free);
	return fails;
}

static int test_lstmap_null()
{
	int fails = 0;
	printf("[TEST] lstmap NULL List\n");

	t_list *mapped = ft_lstmap(NULL, dup_content, del_free);
	if (mapped == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
		ft_lstclear(&mapped, del_free);
	}
	return fails;
}

static void *append_x(void *content)
{
	char *s = (char *)content;
	size_t len = ft_strlen(s);
	char *res = malloc(len + 2);
	if (!res)
		return NULL;
	ft_strlcpy(res, s, len + 2);
	res[len] = 'X';
	res[len + 1] = '\0';
	return res;
}

static int test_lstmap_transforms()
{
	int fails = 0;
	printf("[TEST] lstmap Transforms Content\n");

	t_list *lst = ft_lstnew(ft_strdup("42"));
	t_list *mapped = ft_lstmap(lst, append_x, del_free);

	if (mapped && strcmp(mapped->content, "42X") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	ft_lstclear(&lst, del_free);
	ft_lstclear(&mapped, del_free);
	return fails;
}

static int test_lstmap_independent()
{
	int fails = 0;
	printf("[TEST] lstmap Creates Independent Copy\n");

	t_list *lst = ft_lstnew(ft_strdup("original"));
	t_list *mapped = ft_lstmap(lst, dup_content, del_free);

	if (mapped && mapped->content != lst->content
		&& strcmp(mapped->content, "original") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	ft_lstclear(&lst, del_free);
	ft_lstclear(&mapped, del_free);
	return fails;
}

int testing_ft_lstiter_lstmap()
{
	int fails = 0;

	TITLE("FT_LSTITER");
	sleep(1);
	fails += test_lstiter_count();
	sleep(1);
	fails += test_lstiter_empty();
	sleep(1);
	fails += test_lstiter_modifies();
	sleep(1);

	TITLE("FT_LSTMAP");
	sleep(1);
	fails += test_lstmap_basic();
	sleep(1);
	fails += test_lstmap_null();
	sleep(1);
	fails += test_lstmap_transforms();
	sleep(1);
	fails += test_lstmap_independent();
	sleep(1);

	printf("\n===== Testing Complete =====\n");
	return fails;
}
