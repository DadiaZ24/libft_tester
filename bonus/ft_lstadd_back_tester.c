#include "../tester.h"

static int test_basic_addition()
{
	int fails = 0;
	printf("[TEST] Basic Add to Back\n");

	// Create a list with one element
	t_list *lst = ft_lstnew("A");
	ft_lstadd_back(&lst, ft_lstnew("B"));
	if (lst && lst->next && strcmp(lst->next->content, "B") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_multiple_additions()
{
	int fails = 0;
	printf("[TEST] Multiple Additions\n");

	// Create an empty list
	t_list *lst = NULL;
	ft_lstadd_back(&lst, ft_lstnew("A"));
	ft_lstadd_back(&lst, ft_lstnew("B"));
	ft_lstadd_back(&lst, ft_lstnew("C"));

	// Check the last element
	t_list *last = lst;
	while (last && last->next)
		last = last->next;

	if (last && strcmp(last->content, "C") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

static int test_addition_on_empty_list()
{
	int fails = 0;
	printf("[TEST] Add to Empty List\n");

	// Create an empty list
	t_list *lst = NULL;
	ft_lstadd_back(&lst, ft_lstnew("A"));
	if (lst && strcmp(lst->content, "A") == 0 && !lst->next)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_addition_on_null_list()
{
	int fails = 0;
	printf("[TEST] Add to NULL List\n");

	t_list *lst = NULL;
	ft_lstadd_back(&lst, ft_lstnew("A"));
	if (lst && strcmp(lst->content, "A") == 0 && !lst->next)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_add_to_single_element_list()
{
	int fails = 0;
	printf("[TEST] Add to Single Element List\n");

	// Create a list with a single element
	t_list *lst = ft_lstnew("A");
	ft_lstadd_back(&lst, ft_lstnew("B"));

	if (lst && lst->next && strcmp(lst->next->content, "B") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_add_multiple_elements()
{
	int fails = 0;
	printf("[TEST] Add Multiple Elements\n");

	// Create an empty list
	t_list *lst = NULL;

	// Add multiple elements
	for (char c = 'A'; c <= 'E'; c++)
	{
		char *str = malloc(2);
		if (!str)
			continue; // or handle malloc failure
		str[0] = c;
		str[1] = '\0';
		ft_lstadd_back(&lst, ft_lstnew(str));
	}

	// Check the last element
	t_list *last = lst;
	while (last && last->next)
		last = last->next;

	if (last && strcmp(last->content, "E") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	// Ensure the list order is correct
	t_list *curr = lst;
	char expected[] = "ABCDE";
	int i = 0;
	while (curr)
	{
		if (curr->content && ((char *)curr->content)[0] != expected[i])
		{
			FAIL();
			fails++;
		}
		curr = curr->next;
		i++;
	}
	ft_lstclear(&lst, free);

	return fails;
}

static int test_add_large_data()
{
	int fails = 0;
	printf("[TEST] Add Large Data\n");

	// Create an empty list
	t_list *lst = NULL;

	// Add large data elements
	for (int i = 0; i < 1000; i++)
	{
		char *data = malloc(100);
		snprintf(data, 100, "Data-%d", i);
		ft_lstadd_back(&lst, ft_lstnew(data));
	}

	// Check the last element
	t_list *last = lst;
	while (last && last->next)
		last = last->next;

	if (last && strcmp(last->content, "Data-999") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	ft_lstclear(&lst, free);
	return fails;
}

int testing_ft_lstadd_back()
{
	int fails = 0;
	TITLE("FT_LSTADD_BACK");
	sleep(1);
	fails += test_basic_addition();
	sleep(1);
	fails += test_multiple_additions();
	sleep(1);
	fails += test_addition_on_empty_list();
	sleep(1);
	fails += test_addition_on_null_list();
	sleep(1);
	fails += test_add_to_single_element_list();
	sleep(1);
	fails += test_add_multiple_elements();
	sleep(1);
	fails += test_add_large_data();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}
