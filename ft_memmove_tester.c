#include "tester.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Walloc-size-larger-than="

static int test_basic_move()
{
	int fails = 0;
	printf("[TEST] Basic Move\n");
	char src[] = "Hello, World!";
	char dest[20];
	ft_memmove(dest, src, sizeof(src));
	if (strcmp(src, dest) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_overlap_forward()
{
	int fails = 0;
	printf("[TEST] Overlap Forward\n");
	char buffer[20] = "1234567890";
	ft_memmove(buffer + 2, buffer, 8); // Overlapping move
	if (strncmp(buffer + 2, "12345678", 8) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_overlap_backward()
{
	int fails = 0;
	printf("[TEST] Overlap Backward\n");
	char buffer[20] = "1234567890";
	ft_memmove(buffer, buffer + 2, 8);
	if (strncmp(buffer, "34567890", 8) == 0)
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
	ft_memmove(buffer + 1, buffer, 0); // Should be no-op
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
	memset(src, 'A', size);
	ft_memmove(dest, src, size);
	if (dest[0] == 'A' && dest[size - 1] == 'A')
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

static int test_unaligned_access()
{
	int fails = 0;
	printf("[TEST] Unaligned Access\n");
	char buffer[32];
	for (int i = 0; i < 32; i++)
		buffer[i] = (char)i;
	ft_memmove(buffer + 1, buffer, 31);
	if (buffer[1] == 0 && buffer[2] == 1)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_memmove_within_struct()
{
	int fails = 0;
	printf("[TEST] ft_memmove within struct\n");
	struct
	{
		char a[8];
		char b[8];
	} s = {"ABCDEFGH", "--------"};
	ft_memmove(s.b, s.a, 8);
	if (strncmp(s.b, "ABCDEFGH", 8) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_reversed_move()
{
	int fails = 0;
	printf("[TEST] Reversed Move (src after dest)\n");
	char data[20] = "abcdefghij";
	ft_memmove(data, data + 5, 5);
	if (strncmp(data, "fghij", 5) == 0)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

#pragma GCC diagnostic pop

int testing_ft_memmove()
{
	int fails = 0;
	TITLE("FT_MEMMOVE");
	sleep(1);
	fails += test_basic_move();
	sleep(1);
	fails += test_overlap_forward();
	sleep(1);
	fails += test_overlap_backward();
	sleep(1);
	fails += test_zero_size();
	sleep(1);
	fails += test_large_copy();
	sleep(1);
	fails += test_unaligned_access();
	sleep(1);
	fails += test_memmove_within_struct();
	sleep(1);
	fails += test_reversed_move();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}
