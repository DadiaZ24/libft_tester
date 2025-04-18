#include "tester.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"

static int test_strlcpy_basic()
{
	int fails = 0;
	printf("[TEST] strlcpy Basic\n");
	char dest[20];
	size_t result = ft_strlcpy(dest, "Hello", sizeof(dest));
	if (strcmp(dest, "Hello") == 0 && result == 5)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strlcpy_truncation()
{
	int fails = 0;
	printf("[TEST] strlcpy Truncation\n");
	char dest[5];
	size_t result = ft_strlcpy(dest, "Hello, world!", sizeof(dest));
	if (strncmp(dest, "Hell", 4) == 0 && result == 13)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strlcpy_zero_size()
{
	int fails = 0;
	printf("[TEST] strlcpy Zero Size\n");
	char dest[5] = "test";
	size_t result = ft_strlcpy(dest, "Hello", 0);
	if (strcmp(dest, "test") == 0 && result == 5)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strlcat_basic()
{
	int fails = 0;
	printf("[TEST] strlcat Basic\n");
	char dest[20] = "Hello";
	size_t result = ft_strlcat(dest, ", world", sizeof(dest));
	if (strcmp(dest, "Hello, world") == 0 && result == 12)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strlcat_truncation()
{
	int fails = 0;
	printf("[TEST] strlcat Truncation\n");
	char dest[10] = "Hello";
	size_t result = ft_strlcat(dest, ", world!", sizeof(dest));
	if (strncmp(dest, "Hello, wo", 9) == 0 && result == 13)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strlcat_zero_size()
{
	int fails = 0;
	printf("[TEST] strlcat Zero Size\n");
	char dest[5] = "abcd";
	size_t result = ft_strlcat(dest, "efg", 0);
	if (strcmp(dest, "abcd") == 0 && result == 3)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

#pragma GCC diagnostic pop

int testing_ft_strlcpy_strlcat()
{
	int fails = 0;
	TITLE("FT_STRLCPY & FT_STRLCAT");
	sleep(1);
	fails += test_strlcpy_basic();
	sleep(1);
	fails += test_strlcpy_truncation();
	sleep(1);
	fails += test_strlcpy_zero_size();
	sleep(1);
	fails += test_strlcat_basic();
	sleep(1);
	fails += test_strlcat_truncation();
	sleep(1);
	fails += test_strlcat_zero_size();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}
