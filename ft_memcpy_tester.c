#include "tester.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Walloc-size-larger-than="

static int test_basic_copy()
{
	int fails = 0;
	printf("[TEST] Basic Copy\n");
	char src[] = "Hello, memcpy!";
	char dest[20];
	ft_memcpy(dest, src, sizeof(src));
	if (strcmp(src, dest) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_zero_size()
{
	int fails = 0;
	printf("[TEST] Zero Size\n");
	char buffer[10] = "abc";
	ft_memcpy(buffer + 1, buffer, 0); // Should do nothing
	if (strcmp(buffer, "abc") == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_large_copy()
{
	int fails = 0;
	printf("[TEST] Large Copy\n");
	size_t size = 1 << 26; // 64MB
	char *src = malloc(size);
	char *dest = malloc(size);
	if (!src || !dest)
	{
		printf("  ⚠️  CHECK: Could not allocate memory!!!\n");
		fails++;
		free(src);
		free(dest);
		return fails;
	}
	memset(src, 'B', size);
	ft_memcpy(dest, src, size);
	if (dest[0] == 'B' && dest[size - 1] == 'B')
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free(src);
	free(dest);
	return fails;
}

static int test_no_overlap_allowed()
{
	int fails = 0;
	printf("[TEST] Overlap (Undefined Behavior)\n");
	char data[] = "ABCDEFGHIJ";
	// This is undefined behavior, but we check what ft_memcpy does
	ft_memcpy(data + 2, data, 5);
	// No "correct" result expected, just ensure it doesn't crash
	PASS();
	return fails;
}

static int test_copy_within_struct()
{
	int fails = 0;
	printf("[TEST] Copy within struct\n");
	struct
	{
		char a[8];
		char b[8];
	} s = {"12345678", "--------"};
	ft_memcpy(s.b, s.a, 8);
	if (strncmp(s.b, "12345678", 8) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

#pragma GCC diagnostic pop

int testing_ft_memcpy()
{
	int fails = 0;
	TITLE("FT_MEMCPY");
	sleep(1);
	fails += test_basic_copy();
	sleep(1);
	fails += test_zero_size();
	sleep(1);
	fails += test_large_copy();
	sleep(1);
	fails += test_no_overlap_allowed();
	sleep(1);
	fails += test_copy_within_struct();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}
