#include "tester.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overread"

static int test_strdup_basic()
{
	int fails = 0;
	printf("[TEST] strdup Basic\n");
	char *s = "Hello strdup!";
	char *dup = ft_strdup(s);
	if (dup && strcmp(s, dup) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(dup);
	return fails;
}

static int test_strdup_empty()
{
	int fails = 0;
	printf("[TEST] strdup Empty\n");
	char *dup = ft_strdup("");
	if (dup && strcmp(dup, "") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(dup);
	return fails;
}

static int test_strchr_basic()
{
	int fails = 0;
	printf("[TEST] strchr Basic\n");
	char *s = "abcdef";
	if (ft_strchr(s, 'c') == s + 2)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strchr_not_found()
{
	int fails = 0;
	printf("[TEST] strchr Not Found\n");
	if (ft_strchr("abcdef", 'x') == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strrchr_basic()
{
	int fails = 0;
	printf("[TEST] strrchr Basic\n");
	char *s = "abacad";
	if (ft_strrchr(s, 'a') == s + 4)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strrchr_not_found()
{
	int fails = 0;
	printf("[TEST] strrchr Not Found\n");
	if (ft_strrchr("abcdef", 'z') == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strnstr_basic()
{
	int fails = 0;
	printf("[TEST] strnstr Basic\n");
	char *big = "Hello little world";
	char *little = "little";
	if (ft_strnstr(big, little, 20) == big + 6)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strnstr_limited()
{
	int fails = 0;
	printf("[TEST] strnstr Limited Range\n");
	char *big = "Hello little world";
	char *little = "little";
	if (ft_strnstr(big, little, 5) == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

#pragma GCC diagnostic pop

int testing_ft_string_funcs()
{
	int fails = 0;
	TITLE("FT_STR FUNCTIONS");
	sleep(1);
	fails += test_strdup_basic();
	sleep(1);
	fails += test_strdup_empty();
	sleep(1);
	fails += test_strchr_basic();
	sleep(1);
	fails += test_strchr_not_found();
	sleep(1);
	fails += test_strrchr_basic();
	sleep(1);
	fails += test_strrchr_not_found();
	sleep(1);
	fails += test_strnstr_basic();
	sleep(1);
	fails += test_strnstr_limited();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}
