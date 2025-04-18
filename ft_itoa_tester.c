#include "tester.h"
#include <limits.h>

static int assert_itoa(int input, const char *expected)
{
	int fails = 0;
	char *result = ft_itoa(input);
	if (strcmp(result, expected) == 0)
		PASS();
	else
	{
		FAIL();
		printf("  Input: %d | Expected: \"%s\" | Got: \"%s\"\n", input, expected, result);
		fails++;
	}
	free(result);
	return fails;
}

static int test_zero()
{
	int fails = 0;
	printf("[TEST] Zero\n");
	fails += assert_itoa(0, "0");
	return fails;
}

static int test_positive_values()
{
	int fails = 0;
	printf("[TEST] Positive Integers\n");
	fails += assert_itoa(1, "1");
	fails += assert_itoa(42, "42");
	fails += assert_itoa(214748, "214748");
	return fails;
}

static int test_negative_values()
{
	int fails = 0;
	printf("[TEST] Negative Integers\n");
	fails += assert_itoa(-1, "-1");
	fails += assert_itoa(-42, "-42");
	fails += assert_itoa(-214748, "-214748");
	return fails;
}

static int test_int_limits()
{
	int fails = 0;
	printf("[TEST] INT Limits\n");
	fails += assert_itoa(INT_MAX, "2147483647");
	fails += assert_itoa(INT_MIN, "-2147483648");
	return fails;
}

static int test_palindromes()
{
	int fails = 0;
	printf("[TEST] Palindromic Numbers\n");
	fails += assert_itoa(121, "121");
	fails += assert_itoa(-12321, "-12321");
	return fails;
}

static int test_large_steps()
{
	int fails = 0;
	printf("[TEST] Large Stepping Numbers\n");
	for (int i = 1000000; i <= 10000000; i += 1000000)
	{
		char buffer[20];
		sprintf(buffer, "%d", i);
		fails += assert_itoa(i, buffer);
	}
	return fails;
}

int testing_ft_itoa()
{
	int fails = 0;
	TITLE("FT_ITOA");
	sleep(1);
	fails += test_zero();
	sleep(1);
	fails += test_positive_values();
	sleep(1);
	fails += test_negative_values();
	sleep(1);
	fails += test_int_limits();
	sleep(1);
	fails += test_palindromes();
	sleep(1);
	fails += test_large_steps();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}
