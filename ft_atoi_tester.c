#include "tester.h"
#include <limits.h>

static int test_basic()
{
	int fails = 0;
	printf("[TEST] Basic Values\n");

	if (ft_atoi("0") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	if (ft_atoi("42") == 42)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	if (ft_atoi("-42") == -42)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

static int test_with_spaces()
{
	int fails = 0;
	printf("[TEST] Leading Whitespaces\n");

	if (ft_atoi("   123") == 123)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	if (ft_atoi("\t\n\v\f\r 456") == 456)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

static int test_with_signs()
{
	int fails = 0;
	printf("[TEST] Signs and Double Signs\n");

	if (ft_atoi("+789") == 789)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	if (ft_atoi("-00123") == -123)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	if (ft_atoi("++123") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	if (ft_atoi("--123") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

static int test_invalid_chars()
{
	int fails = 0;
	printf("[TEST] Invalid Characters\n");

	if (ft_atoi("12a3") == 12)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	if (ft_atoi("abc") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	if (ft_atoi("42!@#") == 42)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

static int test_int_limits()
{
	int fails = 0;
	printf("[TEST] INT Limits\n");

	if (ft_atoi("2147483647") == INT_MAX)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	if (ft_atoi("-2147483648") == INT_MIN)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

static int test_trailing_junk()
{
	int fails = 0;
	printf("[TEST] Trailing Junk\n");

	if (ft_atoi("123abc456") == 123)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	if (ft_atoi("  -42xyz") == -42)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

static int test_empty_and_null()
{
	int fails = 0;
	printf("[TEST] Empty and Null Input\n");

	if (ft_atoi("") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	if (ft_atoi(NULL) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}

	return fails;
}

int testing_ft_atoi()
{
	int fails = 0;
	TITLE("FT_ATOI");
	sleep(1);
	fails += test_basic();
	sleep(1);
	fails += test_with_spaces();
	sleep(1);
	fails += test_with_signs();
	sleep(1);
	fails += test_invalid_chars();
	sleep(1);
	fails += test_int_limits();
	sleep(1);
	fails += test_trailing_junk();
	sleep(1);
	fails += test_empty_and_null();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}
