#include "tester.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow="

static int test_striteri_basic()
{
	int fails = 0;
	printf("[TEST] striteri Basic Case\n");
	char str[] = "abc";
	void f(unsigned int i, char *c) { *c += i; }
	ft_striteri(str, f);
	if (strcmp(str, "ace") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_striteri_null()
{
	int fails = 0;
	printf("[TEST] striteri NULL Input\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	ft_striteri(NULL, NULL); // Should not crash
	PASS();
	return fails;
}

static int test_bzero_basic()
{
	int fails = 0;
	printf("[TEST] bzero Basic Case\n");
	char buffer[10] = "abcdefghi";
	ft_bzero(buffer, 5);
	int zeroed = 1;
	for (int i = 0; i < 5; ++i)
		if (buffer[i] != 0)
			zeroed = 0;
	if (zeroed)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_bzero_zero_len()
{
	int fails = 0;
	printf("[TEST] bzero Zero Length\n");
	char buffer[] = "abc";
	ft_bzero(buffer, 0);
	if (strcmp(buffer, "abc") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strncmp_equal()
{
	int fails = 0;
	printf("[TEST] strncmp Equal Strings\n");
	if (ft_strncmp("abc", "abc", 3) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strncmp_diff()
{
	int fails = 0;
	printf("[TEST] strncmp Different Strings\n");
	if (ft_strncmp("abc", "abd", 3) < 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strncmp_partial()
{
	int fails = 0;
	printf("[TEST] strncmp Partial Compare\n");
	if (ft_strncmp("abcdef", "abcxyz", 3) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

int testing_ft_striteri_bzero_strncmp()
{
	int fails = 0;
	TITLE("FT_STRITERI / FT_BZERO / FT_STRNCMP");
	sleep(1);
	fails += test_striteri_basic();
	sleep(1);
	fails += test_striteri_null();
	sleep(1);
	fails += test_bzero_basic();
	sleep(1);
	fails += test_bzero_zero_len();
	sleep(1);
	fails += test_strncmp_equal();
	sleep(1);
	fails += test_strncmp_diff();
	sleep(1);
	fails += test_strncmp_partial();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}

#pragma GCC diagnostic pop