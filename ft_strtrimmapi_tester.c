#include "tester.h"

// FT_STRTRIM TESTS
static int test_strtrim_basic()
{
	int fails = 0;
	printf("[TEST] strtrim Basic\n");
	char *res = ft_strtrim("  Hello World  ", " ");
	if (strcmp(res, "Hello World") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(res);
	return fails;
}

static int test_strtrim_all_trimmed()
{
	int fails = 0;
	printf("[TEST] strtrim All Trimmed\n");
	char *res = ft_strtrim("xxxx", "x");
	if (strcmp(res, "") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(res);
	return fails;
}

// FT_SUBSTR TESTS
static int test_substr_basic()
{
	int fails = 0;
	printf("[TEST] substr Basic\n");
	char *res = ft_substr("Hello World", 6, 5);
	if (strcmp(res, "World") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(res);
	return fails;
}

static int test_substr_start_beyond()
{
	int fails = 0;
	printf("[TEST] substr Start Beyond\n");
	char *res = ft_substr("Hello", 10, 3);
	if (strcmp(res, "") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(res);
	return fails;
}

// FT_STRMAPI TESTS
static char map_to_upper(unsigned int i, char c)
{
	(void)i;
	if (c >= 'a' && c <= 'z')
		return c - 32;
	return c;
}

static int test_strmapi_basic()
{
	int fails = 0;
	printf("[TEST] strmapi Basic\n");
	char *res = ft_strmapi("hello", map_to_upper);
	if (strcmp(res, "HELLO") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(res);
	return fails;
}

int testing_ft_strl()
{
	int fails = 0;

	TITLE("FT_STRTRIM");
	fails += test_strtrim_basic();
	sleep(1);
	fails += test_strtrim_all_trimmed();
	sleep(1);

	TITLE("FT_SUBSTR");
	fails += test_substr_basic();
	sleep(1);
	fails += test_substr_start_beyond();
	sleep(1);

	TITLE("FT_STRMAPI");
	fails += test_strmapi_basic();
	sleep(1);

	printf("\n===== Testing Complete =====\n");
	return fails;
}