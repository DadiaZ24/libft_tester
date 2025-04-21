#include "tester.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Walloc-size-larger-than="

static int test_integer_overflow()
{
	int fails = 0;

	printf("[TEST] Integer Overflow\n");
	size_t a = SIZE_MAX / 2 + 1;
	size_t b = 2;
	void *ptr = ft_calloc(a, b);
	if (!ptr)
		PASS();
	else
	{
		FAIL();
		fails++;
		free(ptr);
	}
	return fails;
}

static int test_size_overflow()
{
	int fails = 0;

	printf("[TEST] SIZE Overflow\n");
	void *ptr = ft_calloc((size_t)-1, sizeof(int));
	if (!ptr)
		PASS();
	else
	{
		FAIL();
		fails++;
		free(ptr);
	}
	return fails;
}

static int test_zero_allocation()
{
	int fails = 0;

	printf("[TEST] Zero Allocation\n");
	void *ptr1 = ft_calloc(0, 1000);
	void *ptr2 = ft_calloc(1000, 0);

	if (ptr1 || ptr2)
		PASS(); // valid: implementation-defined
	else
		PASS(); // also valid: NULL on zero alloc
	free(ptr1);
	free(ptr2);
	return fails;
}

static int test_maximum_allocation()
{
	int fails = 0;

	printf("[TEST] Maximum Allocation\n");
	void *ptr = ft_calloc(1, SIZE_MAX);
	if (!ptr)
		PASS();
	else
	{
		FAIL();
		fails++;
		free(ptr);
	}
	return fails;
}

static int test_fuzz_allocations()
{
	int fails = 0;

	printf("[TEST] Fuzzing Increasing Sizes (limited)\n");
	size_t step = 1 << 20;

	for (size_t i = 1; i <= 5000; i *= 2)
	{
		void *ptr = ft_calloc(i, step);
		if (!ptr)
		{
			printf("  Reached alloc limit at %zu x %zu\n", i, step);
			fails++;
			break;
		}
		else
		{
			free(ptr);
		}
	}
	PASS();
	return (fails);
}

static int test_negative_size_cast()
{
	int fails = 0;

	printf("[TEST] Negative Size Cast\n");
	int neg = -10;
	void *ptr = ft_calloc((size_t)neg, sizeof(int));
	if (!ptr)
		PASS();
	else
	{
		printf("  CHECK: calloc returned non-null for size = %zu\n", (size_t)neg);
		fails++;
		CHECK();
		free(ptr);
	}
	return fails;
}

static void *thread_calloc_test(void *arg)
{
	(void)arg;
	for (int i = 0; i < ALLOCS_PER_THREAD; ++i)
	{
		void *ptr = ft_calloc(10, 10);
		if (ptr)
			free(ptr);
	}
	return NULL;
}

static int test_multithreaded_calloc()
{
	int fails = 0;
	printf("[TEST] Multithreaded calloc abuse\n");
	pthread_t threads[NUM_THREADS];
	int error = 0;

	for (int i = 0; i < NUM_THREADS; ++i)
		if (pthread_create(&threads[i], NULL, thread_calloc_test, NULL) != 0)
			error = 1;

	for (int i = 0; i < NUM_THREADS; ++i)
		pthread_join(threads[i], NULL);

	if (!error)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return fails;
}

static int test_use_after_free()
{
	int fails = 0;
	printf("[TEST] Use-After-Free (intentional UB)\n");
	int *arr = ft_calloc(4, sizeof(int));
	if (!arr)
	{
		FAIL();
		fails++;
		return fails;
	}
	free(arr);
	arr[0] = 42; // UAF (undefined behavior)
	printf("  CHECK: Wrote to freed memory (value: %d)\n", arr[0]);
	PASS();
	return fails;
}

#pragma GCC diagnostic pop

int testing_ft_calloc()
{
	int fails = 0;

	TITLE("FT_CALLOC");
	fails += test_integer_overflow();
	sleep(1);
	fails += test_size_overflow();
	sleep(1);
	fails += test_zero_allocation();
	sleep(1);
	fails += test_maximum_allocation();
	sleep(1);
	fails += test_fuzz_allocations();
	sleep(1);
	fails += test_negative_size_cast();
	sleep(1);
	fails += test_multithreaded_calloc();
	sleep(1);
	fails += test_use_after_free();
	sleep(1);
	printf("\n===== Testing Complete =====\n");
	return fails;
}
