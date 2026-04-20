#include "../tester.h"

// ===== FT_LSTNEW =====

static int test_lstnew_basic()
{
	int fails = 0;
	printf("[TEST] lstnew Basic\n");
	t_list *node = ft_lstnew("hello");
	if (node && strcmp(node->content, "hello") == 0 && node->next == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(node);
	return fails;
}

static int test_lstnew_null_content()
{
	int fails = 0;
	printf("[TEST] lstnew NULL Content\n");
	t_list *node = ft_lstnew(NULL);
	if (node && node->content == NULL && node->next == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(node);
	return fails;
}

static int test_lstnew_int_content()
{
	int fails = 0;
	printf("[TEST] lstnew Integer Content\n");
	int val = 42;
	t_list *node = ft_lstnew(&val);
	if (node && *(int *)node->content == 42 && node->next == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(node);
	return fails;
}

// ===== FT_LSTSIZE =====

static int test_lstsize_empty()
{
	int fails = 0;
	printf("[TEST] lstsize Empty List\n");
	if (ft_lstsize(NULL) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_lstsize_one()
{
	int fails = 0;
	printf("[TEST] lstsize Single Node\n");
	t_list *lst = ft_lstnew("a");
	if (ft_lstsize(lst) == 1)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(lst);
	return fails;
}

static int test_lstsize_multiple()
{
	int fails = 0;
	printf("[TEST] lstsize Multiple Nodes\n");
	t_list *lst = ft_lstnew("a");
	ft_lstadd_back(&lst, ft_lstnew("b"));
	ft_lstadd_back(&lst, ft_lstnew("c"));
	ft_lstadd_back(&lst, ft_lstnew("d"));
	if (ft_lstsize(lst) == 4)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	ft_lstclear(&lst, NULL);
	return fails;
}

// ===== FT_LSTDELONE =====

static void del_content(void *content)
{
	free(content);
}

static int test_lstdelone_basic()
{
	int fails = 0;
	printf("[TEST] lstdelone Basic (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);

	char *str = malloc(6);
	if (!str)
		return fails;
	ft_strlcpy(str, "hello", 6);
	t_list *node = ft_lstnew(str);
	t_list *next = ft_lstnew("world");
	node->next = next;

	ft_lstdelone(node, del_content);
	if (strcmp(next->content, "world") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(next);
	return fails;
}

static int test_lstdelone_null_del()
{
	int fails = 0;
	printf("[TEST] lstdelone NULL del function (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);

	t_list *node = ft_lstnew("static string");
	ft_lstdelone(node, NULL);
	PASS();
	return fails;
}

int testing_ft_lstnew_lstsize_lstdelone()
{
	int fails = 0;

	TITLE("FT_LSTNEW");
	sleep(1);
	fails += test_lstnew_basic();
	sleep(1);
	fails += test_lstnew_null_content();
	sleep(1);
	fails += test_lstnew_int_content();
	sleep(1);

	TITLE("FT_LSTSIZE");
	sleep(1);
	fails += test_lstsize_empty();
	sleep(1);
	fails += test_lstsize_one();
	sleep(1);
	fails += test_lstsize_multiple();
	sleep(1);

	TITLE("FT_LSTDELONE");
	sleep(1);
	fails += test_lstdelone_basic();
	sleep(1);
	fails += test_lstdelone_null_del();
	sleep(1);

	printf("\n===== Testing Complete =====\n");
	return fails;
}
