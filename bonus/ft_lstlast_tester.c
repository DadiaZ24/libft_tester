#include "../tester.h"

static int test_lst_last_basic()
{
	int fails = 0;
	printf("[TEST] lst_last Basic\n");

	t_list node1 = {.content = "First", .next = NULL};
	t_list node2 = {.content = "Second", .next = NULL};
	t_list node3 = {.content = "Last", .next = NULL};
	node1.next = &node2;
	node2.next = &node3;

	t_list *last = ft_lstlast(&node1);

	if (last && last->content == "Last")
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

static int test_lst_last_empty()
{
	int fails = 0;
	printf("[TEST] ft_lstlast Empty List\n");

	t_list *last = ft_lstlast(NULL);

	if (last == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

static int test_lst_last_one_element()
{
	int fails = 0;
	printf("[TEST] ft_lstlast One Element\n");

	t_list node1 = {.content = "Only", .next = NULL};
	t_list *last = ft_lstlast(&node1);

	if (last && last->content == "Only")
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

static int test_lst_last_multiple_elements()
{
	int fails = 0;
	printf("[TEST] ft_lstlast Multiple Elements\n");

	t_list node1 = {.content = "First", .next = NULL};
	t_list node2 = {.content = "Second", .next = NULL};
	t_list node3 = {.content = "Last", .next = NULL};
	node1.next = &node2;
	node2.next = &node3;

	t_list *last = ft_lstlast(&node1);

	if (last && last->content == "Last")
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

static int test_lst_last_multiple_elements_with_cycles()
{
	int fails = 0;
	printf("[TEST] ft_lstlast Multiple Elements with Cycle (Edge Case)\n");

	t_list node1 = {.content = "First", .next = NULL};
	t_list node2 = {.content = "Second", .next = NULL};
	t_list node3 = {.content = "Last", .next = NULL};
	node1.next = &node2;
	node2.next = &node3;
	node3.next = &node1; // Creating a cycle

	printf("\e[0;31m -- IF YOU ARE IN AN INFINITE LOOP, TO BAD, YOU HAVE NOT PASSED THIS TEST! -- \e[0m\n");

	// Here ft_lstlast should not be able to work properly due to the cycle
	t_list *last = ft_lstlast(&node1);

	// In case of a cycle, ft_lstlast should return NULL or break out somehow, if properly implemented
	if (last == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

static int test_lst_last_large_list()
{
	int fails = 0;
	printf("[TEST] ft_lstlast Large List\n");

	t_list *head = NULL;
	t_list *temp = NULL;
	for (int i = 0; i < 10000; i++)
	{
		temp = malloc(sizeof(t_list));
		if (!temp)
		{
			printf("  ⚠️  Memory allocation failed during large list test\n");
			fails++;
			break;
		}
		temp->content = "Data";
		temp->next = head;
		head = temp;
	}

	t_list *last = ft_lstlast(head);
	if (last)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	// Clean up large list
	while (head)
	{
		temp = head;
		head = head->next;
		free(temp);
	}

	return fails;
}

int testing_ft_lst_last()
{
	int fails = 0;
	TITLE("ft_lstlast");
	sleep(1);
	fails += test_lst_last_basic();
	sleep(1);
	fails += test_lst_last_empty();
	sleep(1);
	fails += test_lst_last_one_element();
	sleep(1);
	fails += test_lst_last_multiple_elements();
	sleep(1);
	fails += test_lst_last_multiple_elements_with_cycles();
	sleep(1);
	fails += test_lst_last_large_list();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}
