#include "tester.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overread"

static int test_split_basic()
{
	int fails = 0;
	printf("[TEST] split Basic\n");
	char **result = ft_split("a,b,c", ',');
	if (result && strcmp(result[0], "a") == 0 && strcmp(result[1], "b") == 0 && strcmp(result[2], "c") == 0 && result[3] == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	int i = -1;
	while (result[--i])
		free(result[i]);
	free(result);
	return fails;
}

static int test_split_empty_string()
{
	int fails = 0;
	printf("[TEST] split Empty String\n");
	char **result = ft_split("", ',');
	if (result && result[0] == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(result);
	return fails;
}

static int test_split_only_delimiters()
{
	int fails = 0;
	printf("[TEST] split Only Delimiters\n");
	char **result = ft_split(",,,", ',');
	if (result && result[0] == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(result);
	return fails;
}

static int test_split_no_delimiters()
{
	int fails = 0;
	printf("[TEST] split No Delimiters\n");
	char **result = ft_split("abc", ',');
	if (result && strcmp(result[0], "abc") == 0 && result[1] == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(result[0]);
	free(result);
	return fails;
}

static int test_split_long_string()
{
	int fails = 0;
	printf("[TEST] split Long String\n");
	char long_str[1024];
	for (int i = 0; i < 1023; i++)
		long_str[i] = (i % 10 == 0) ? ',' : 'x';
	long_str[1023] = '\0';
	char **result = ft_split(long_str, ',');
	if (result && result[0] && strlen(result[0]) > 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	for (int i = 0; result && result[i]; i++)
		free(result[i]);
	free(result);
	return fails;
}

#pragma GCC diagnostic pop

int testing_ft_split()
{
	int fails = 0;
	TITLE("FT_SPLIT");
	sleep(1);
	fails += test_split_basic();
	sleep(1);
	fails += test_split_empty_string();
	sleep(1);
	fails += test_split_only_delimiters();
	sleep(1);
	fails += test_split_no_delimiters();
	sleep(1);
	fails += test_split_long_string();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}
