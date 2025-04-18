#include "../tester.h"

static void *alloc_content(const char *s)
{
	char *dup = strdup(s);
	if (!dup)
	{
		printf("  ⚠️  Allocation failed\n");
	}
	return dup;
}

static int test_clear_null_list()
{
	int fails = 0;
	printf("[TEST] Clear NULL List\n");

	t_list *lst = NULL;
	ft_lstclear(&lst, free);

	if (lst == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_clear_single_element()
{
	int fails = 0;
	printf("[TEST] Clear Single Element\n");

	t_list *lst = ft_lstnew(alloc_content("One"));
	ft_lstclear(&lst, free);

	if (lst == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_clear_multiple_elements()
{
	int fails = 0;
	printf("[TEST] Clear Multiple Elements\n");

	t_list *lst = NULL;
	ft_lstadd_back(&lst, ft_lstnew(alloc_content("One")));
	ft_lstadd_back(&lst, ft_lstnew(alloc_content("Two")));
	ft_lstadd_back(&lst, ft_lstnew(alloc_content("Three")));

	ft_lstclear(&lst, free);

	if (lst == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_clear_with_custom_free()
{
	int fails = 0;
	printf("[TEST] Clear with Custom Free Function\n");

	void custom_free(void *ptr)
	{
		if (ptr)
			memset(ptr, 0, strlen(ptr)); // wipe data before free
		free(ptr);
	}

	t_list *lst = NULL;
	ft_lstadd_back(&lst, ft_lstnew(alloc_content("Secret1")));
	ft_lstadd_back(&lst, ft_lstnew(alloc_content("Secret2")));
	ft_lstadd_back(&lst, ft_lstnew(alloc_content("Secret3")));

	ft_lstclear(&lst, custom_free);

	if (lst == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_clear_and_access()
{
	int fails = 0;
	printf("[TEST] Clear Then Access (Use-After-Free Protection)\n");

	t_list *lst = NULL;
	t_list *node = ft_lstnew(alloc_content("Don't touch me"));
	ft_lstadd_back(&lst, node);

	ft_lstclear(&lst, free);

	// Should not access anymore — but we check if list was correctly cleared
	if (lst == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

int testing_ft_lstclear()
{
	int fails = 0;
	TITLE("FT_LSTCLEAR");
	sleep(1);
	fails += test_clear_null_list();
	sleep(1);
	fails += test_clear_single_element();
	sleep(1);
	fails += test_clear_multiple_elements();
	sleep(1);
	fails += test_clear_with_custom_free();
	sleep(1);
	fails += test_clear_and_access();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}
