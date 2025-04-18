#include "tester.h"

static int test_isalpha_basic()
{
	int fails = 0;
	printf("[TEST] isalpha Basic\n");
	if (ft_isalpha('a') && ft_isalpha('Z') && !ft_isalpha('1'))
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_isdigit_basic()
{
	int fails = 0;
	printf("[TEST] isdigit Basic\n");
	if (ft_isdigit('5') && !ft_isdigit('a'))
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_toupper_basic()
{
	int fails = 0;
	printf("[TEST] toupper Basic\n");
	if (ft_toupper('a') == 'A' && ft_toupper('Z') == 'Z')
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_tolower_basic()
{
	int fails = 0;
	printf("[TEST] tolower Basic\n");
	if (ft_tolower('A') == 'a' && ft_tolower('z') == 'z')
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_isalpha_edge()
{
	int fails = 0;
	printf("[TEST] isalpha Edge Case\n");
	if (!ft_isalpha('1') && !ft_isalpha('!'))
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_isdigit_edge()
{
	int fails = 0;
	printf("[TEST] isdigit Edge Case\n");
	if (ft_isdigit('0') && !ft_isdigit('$'))
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_toupper_edge()
{
	int fails = 0;
	printf("[TEST] toupper Edge Case\n");
	if (ft_toupper('!') == '!')
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_tolower_edge()
{
	int fails = 0;
	printf("[TEST] tolower Edge Case\n");
	if (ft_tolower('!') == '!')
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

int testing_ft_isalpha_isdigit_toupper_tolower()
{
	int fails = 0;
	TITLE("FT_ISALPHA / FT_ISDIGIT / FT_TOUPPER / FT_TOLOWER");
	sleep(1);
	fails += test_isalpha_basic();
	sleep(1);
	fails += test_isdigit_basic();
	sleep(1);
	fails += test_toupper_basic();
	sleep(1);
	fails += test_tolower_basic();
	sleep(1);
	fails += test_isalpha_edge();
	sleep(1);
	fails += test_isdigit_edge();
	sleep(1);
	fails += test_toupper_edge();
	sleep(1);
	fails += test_tolower_edge();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}