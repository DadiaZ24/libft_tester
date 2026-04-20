#include "tester.h"

// ===== FT_STRLEN =====

static int test_strlen_basic()
{
	int fails = 0;
	printf("[TEST] strlen Basic\n");
	if (ft_strlen("Hello") == 5)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strlen_empty()
{
	int fails = 0;
	printf("[TEST] strlen Empty String\n");
	if (ft_strlen("") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strlen_long()
{
	int fails = 0;
	printf("[TEST] strlen Long String\n");
	if (ft_strlen("abcdefghijklmnopqrstuvwxyz") == 26)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_strlen_with_spaces()
{
	int fails = 0;
	printf("[TEST] strlen With Spaces\n");
	if (ft_strlen("hello world") == 11)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

// ===== FT_MEMSET =====

static int test_memset_basic()
{
	int fails = 0;
	printf("[TEST] memset Basic\n");
	char buf[10];
	ft_memset(buf, 'x', 10);
	int ok = 1;
	for (int i = 0; i < 10; i++)
		if (buf[i] != 'x')
			ok = 0;
	if (ok)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_memset_zero()
{
	int fails = 0;
	printf("[TEST] memset Zero Fill\n");
	char buf[5] = "abcde";
	ft_memset(buf, 0, 5);
	int ok = 1;
	for (int i = 0; i < 5; i++)
		if (buf[i] != 0)
			ok = 0;
	if (ok)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_memset_zero_len()
{
	int fails = 0;
	printf("[TEST] memset Zero Length\n");
	char buf[] = "abc";
	ft_memset(buf, 'z', 0);
	if (buf[0] == 'a')
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_memset_partial()
{
	int fails = 0;
	printf("[TEST] memset Partial Fill\n");
	char buf[6] = "aaaaaa";
	ft_memset(buf, 'b', 3);
	if (buf[0] == 'b' && buf[1] == 'b' && buf[2] == 'b'
		&& buf[3] == 'a' && buf[4] == 'a')
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

// ===== FT_MEMCHR =====

static int test_memchr_found()
{
	int fails = 0;
	printf("[TEST] memchr Found\n");
	char buf[] = "abcdef";
	char *res = ft_memchr(buf, 'c', 6);
	if (res == buf + 2)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_memchr_not_found()
{
	int fails = 0;
	printf("[TEST] memchr Not Found\n");
	char buf[] = "abcdef";
	char *res = ft_memchr(buf, 'z', 6);
	if (res == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_memchr_limited()
{
	int fails = 0;
	printf("[TEST] memchr Limited Range\n");
	char buf[] = "abcdef";
	char *res = ft_memchr(buf, 'd', 3);
	if (res == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_memchr_first_byte()
{
	int fails = 0;
	printf("[TEST] memchr First Byte\n");
	char buf[] = "abc";
	char *res = ft_memchr(buf, 'a', 3);
	if (res == buf)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

// ===== FT_MEMCMP =====

static int test_memcmp_equal()
{
	int fails = 0;
	printf("[TEST] memcmp Equal Blocks\n");
	if (ft_memcmp("abc", "abc", 3) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_memcmp_less()
{
	int fails = 0;
	printf("[TEST] memcmp Less Than\n");
	if (ft_memcmp("abc", "abd", 3) < 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_memcmp_greater()
{
	int fails = 0;
	printf("[TEST] memcmp Greater Than\n");
	if (ft_memcmp("abd", "abc", 3) > 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_memcmp_zero_len()
{
	int fails = 0;
	printf("[TEST] memcmp Zero Length\n");
	if (ft_memcmp("abc", "xyz", 0) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_memcmp_partial()
{
	int fails = 0;
	printf("[TEST] memcmp Partial Compare\n");
	if (ft_memcmp("abcXXX", "abcYYY", 3) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

int testing_ft_strlen_memset_memchr_memcmp()
{
	int fails = 0;

	TITLE("FT_STRLEN");
	sleep(1);
	fails += test_strlen_basic();
	sleep(1);
	fails += test_strlen_empty();
	sleep(1);
	fails += test_strlen_long();
	sleep(1);
	fails += test_strlen_with_spaces();
	sleep(1);

	TITLE("FT_MEMSET");
	sleep(1);
	fails += test_memset_basic();
	sleep(1);
	fails += test_memset_zero();
	sleep(1);
	fails += test_memset_zero_len();
	sleep(1);
	fails += test_memset_partial();
	sleep(1);

	TITLE("FT_MEMCHR");
	sleep(1);
	fails += test_memchr_found();
	sleep(1);
	fails += test_memchr_not_found();
	sleep(1);
	fails += test_memchr_limited();
	sleep(1);
	fails += test_memchr_first_byte();
	sleep(1);

	TITLE("FT_MEMCMP");
	sleep(1);
	fails += test_memcmp_equal();
	sleep(1);
	fails += test_memcmp_less();
	sleep(1);
	fails += test_memcmp_greater();
	sleep(1);
	fails += test_memcmp_zero_len();
	sleep(1);
	fails += test_memcmp_partial();
	sleep(1);

	printf("\n===== Testing Complete =====\n");
	return fails;
}
