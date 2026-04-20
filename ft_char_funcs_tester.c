#include "tester.h"

static int test_isalnum_basic()
{
	int fails = 0;
	printf("[TEST] isalnum Basic\n");
	if (ft_isalnum('a') == 1 && ft_isalnum('Z') == 1 && ft_isalnum('5') == 1)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_isalnum_non_alnum()
{
	int fails = 0;
	printf("[TEST] isalnum Non-Alphanumeric\n");
	if (ft_isalnum('!') == 0 && ft_isalnum(' ') == 0 && ft_isalnum('\n') == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_isalnum_boundaries()
{
	int fails = 0;
	printf("[TEST] isalnum Boundary Characters\n");
	if (ft_isalnum('0') == 1 && ft_isalnum('9') == 1
		&& ft_isalnum('a') == 1 && ft_isalnum('z') == 1
		&& ft_isalnum('A') == 1 && ft_isalnum('Z') == 1)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_isascii_basic()
{
	int fails = 0;
	printf("[TEST] isascii Basic\n");
	if (ft_isascii(0) == 1 && ft_isascii(127) == 1 && ft_isascii('A') == 1)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_isascii_out_of_range()
{
	int fails = 0;
	printf("[TEST] isascii Out of Range\n");
	if (ft_isascii(128) == 0 && ft_isascii(255) == 0 && ft_isascii(-1) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_isascii_boundaries()
{
	int fails = 0;
	printf("[TEST] isascii Boundary Values\n");
	if (ft_isascii(0) == 1 && ft_isascii(127) == 1
		&& ft_isascii(126) == 1 && ft_isascii(1) == 1)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_isprint_basic()
{
	int fails = 0;
	printf("[TEST] isprint Basic\n");
	if (ft_isprint('A') == 1 && ft_isprint(' ') == 1 && ft_isprint('~') == 1)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_isprint_non_printable()
{
	int fails = 0;
	printf("[TEST] isprint Non-Printable\n");
	if (ft_isprint('\n') == 0 && ft_isprint('\t') == 0
		&& ft_isprint(0) == 0 && ft_isprint(31) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_isprint_boundaries()
{
	int fails = 0;
	printf("[TEST] isprint Boundary Values\n");
	if (ft_isprint(32) == 1 && ft_isprint(126) == 1
		&& ft_isprint(31) == 0 && ft_isprint(127) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

int testing_ft_isalnum_isascii_isprint()
{
	int fails = 0;
	TITLE("FT_ISALNUM / FT_ISASCII / FT_ISPRINT");
	sleep(1);
	fails += test_isalnum_basic();
	sleep(1);
	fails += test_isalnum_non_alnum();
	sleep(1);
	fails += test_isalnum_boundaries();
	sleep(1);
	fails += test_isascii_basic();
	sleep(1);
	fails += test_isascii_out_of_range();
	sleep(1);
	fails += test_isascii_boundaries();
	sleep(1);
	fails += test_isprint_basic();
	sleep(1);
	fails += test_isprint_non_printable();
	sleep(1);
	fails += test_isprint_boundaries();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}
