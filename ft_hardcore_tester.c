#include "tester.h"
#include <dlfcn.h>

/*
** MALLOC INTERPOSER
** real_malloc is resolved at library load time via __attribute__((constructor)),
** which runs before main(). At that point the dynamic linker is fully ready so
** dlsym is safe. This avoids the lazy-init race where dlsym itself calls
** malloc before real_malloc is set, corrupting the heap.
**
** set_malloc_fail_after(n) : only n more mallocs succeed; the (n+1)th → NULL.
** set_malloc_fail_after(-1): unlimited (default).
*/

static void	*(*real_malloc)(size_t) = NULL;
static int	g_allocs_remaining = -1;

__attribute__((constructor))
static void	init_malloc_hook(void)
{
	real_malloc = dlsym(RTLD_NEXT, "malloc");
}

void	set_malloc_fail_after(int n)
{
	g_allocs_remaining = n;
}

void	*malloc(size_t size)
{
	if (g_allocs_remaining == 0)
		return (NULL);
	if (g_allocs_remaining > 0)
		g_allocs_remaining--;
	return (real_malloc(size));
}

/* ------------------------------------------------------------------ */
/* Helper: free a NULL-terminated string array from ft_split          */
/* ------------------------------------------------------------------ */
static void	free_split(char **arr)
{
	int	i;

	if (!arr)
		return ;
	i = 0;
	while (arr[i])
		free(arr[i++]);
	free(arr);
}

/* ------------------------------------------------------------------ */
/* FT_SPLIT HARDCORE TESTS                                            */
/* ------------------------------------------------------------------ */

static int	test_split_normal()
{
	int		fails = 0;
	char	**res;

	printf("[HARDCORE] ft_split Normal (unlimited malloc)\n");
	set_malloc_fail_after(-1);
	res = ft_split("hello world foo bar", ' ');
	if (!res)
	{
		FAIL();
		return (++fails);
	}
	if (strcmp(res[0], "hello") || strcmp(res[1], "world")
		|| strcmp(res[2], "foo") || strcmp(res[3], "bar") || res[4])
	{
		FAIL();
		fails++;
	}
	else
		PASS();
	free_split(res);
	return (fails);
}

static int	test_split_malloc_fail_at_0()
{
	int		fails = 0;
	char	**res;

	printf("[HARDCORE] ft_split malloc fails immediately (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	set_malloc_fail_after(0);
	res = ft_split("hello world foo bar", ' ');
	set_malloc_fail_after(-1);
	if (res == NULL)
		PASS();
	else
	{
		free_split(res);
		FAIL();
		fails++;
	}
	return (fails);
}

static int	test_split_malloc_fail_after_array()
{
	int		fails = 0;
	char	**res;

	printf("[HARDCORE] ft_split malloc fails after array alloc (no crash + no leak)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	/*
	** ft_split("a b c d", ' ') needs: 1 alloc for the array + 4 for the strings.
	** Allow only the array allocation to succeed -> strings fail -> must return NULL.
	*/
	set_malloc_fail_after(1);
	res = ft_split("a b c d", ' ');
	set_malloc_fail_after(-1);
	if (res == NULL)
		PASS();
	else
	{
		free_split(res);
		CHECK();
	}
	return (fails);
}

static int	test_split_malloc_fail_mid_words()
{
	int		fails = 0;
	char	**res;

	printf("[HARDCORE] ft_split malloc fails after 2 words (no crash + no leak)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	/*
	** Allow array + 2 word allocs to succeed, then fail on word 3.
	** A protected split must free everything and return NULL.
	*/
	set_malloc_fail_after(3);
	res = ft_split("one two three four five", ' ');
	set_malloc_fail_after(-1);
	if (res == NULL)
		PASS();
	else
	{
		free_split(res);
		CHECK();
	}
	return (fails);
}

static int	test_split_empty_string()
{
	int		fails = 0;
	char	**res;

	printf("[HARDCORE] ft_split Empty String\n");
	set_malloc_fail_after(-1);
	res = ft_split("", ' ');
	if (res && res[0] == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free_split(res);
	return (fails);
}

static int	test_split_only_delimiters()
{
	int		fails = 0;
	char	**res;

	printf("[HARDCORE] ft_split Only Delimiters\n");
	set_malloc_fail_after(-1);
	res = ft_split("   ", ' ');
	if (res && res[0] == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free_split(res);
	return (fails);
}

static int	test_split_no_delimiter()
{
	int		fails = 0;
	char	**res;

	printf("[HARDCORE] ft_split No Delimiter Found\n");
	set_malloc_fail_after(-1);
	res = ft_split("hello", ' ');
	if (res && strcmp(res[0], "hello") == 0 && res[1] == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free_split(res);
	return (fails);
}

static int	test_split_consecutive_delimiters()
{
	int		fails = 0;
	char	**res;

	printf("[HARDCORE] ft_split Consecutive Delimiters\n");
	set_malloc_fail_after(-1);
	res = ft_split("a,,b,,c", ',');
	if (res && strcmp(res[0], "a") == 0
		&& strcmp(res[1], "b") == 0
		&& strcmp(res[2], "c") == 0
		&& res[3] == NULL)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	free_split(res);
	return (fails);
}

/* ------------------------------------------------------------------ */
/* FT_SUBSTR HARDCORE                                                 */
/* ------------------------------------------------------------------ */

static int	test_substr_malloc_fail()
{
	int		fails = 0;
	char	*res;

	printf("[HARDCORE] ft_substr malloc fails (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	set_malloc_fail_after(0);
	res = ft_substr("Hello World", 0, 5);
	set_malloc_fail_after(-1);
	if (res == NULL)
		PASS();
	else
	{
		free(res);
		FAIL();
		fails++;
	}
	return (fails);
}

/* ------------------------------------------------------------------ */
/* FT_STRJOIN HARDCORE                                                */
/* ------------------------------------------------------------------ */

static int	test_strjoin_malloc_fail()
{
	int		fails = 0;
	char	*res;

	printf("[HARDCORE] ft_strjoin malloc fails (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	set_malloc_fail_after(0);
	res = ft_strjoin("Hello", " World");
	set_malloc_fail_after(-1);
	if (res == NULL)
		PASS();
	else
	{
		free(res);
		FAIL();
		fails++;
	}
	return (fails);
}

/* ------------------------------------------------------------------ */
/* FT_STRTRIM HARDCORE                                                */
/* ------------------------------------------------------------------ */

static int	test_strtrim_malloc_fail()
{
	int		fails = 0;
	char	*res;

	printf("[HARDCORE] ft_strtrim malloc fails (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	set_malloc_fail_after(0);
	res = ft_strtrim("  hello  ", " ");
	set_malloc_fail_after(-1);
	if (res == NULL)
		PASS();
	else
	{
		free(res);
		FAIL();
		fails++;
	}
	return (fails);
}

/* ------------------------------------------------------------------ */
/* FT_STRMAPI HARDCORE                                                */
/* ------------------------------------------------------------------ */

static char	identity(unsigned int i, char c) { (void)i; return (c); }

static int	test_strmapi_malloc_fail()
{
	int		fails = 0;
	char	*res;

	printf("[HARDCORE] ft_strmapi malloc fails (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	set_malloc_fail_after(0);
	res = ft_strmapi("hello", identity);
	set_malloc_fail_after(-1);
	if (res == NULL)
		PASS();
	else
	{
		free(res);
		FAIL();
		fails++;
	}
	return (fails);
}

/* ------------------------------------------------------------------ */
/* FT_ITOA HARDCORE                                                   */
/* ------------------------------------------------------------------ */

static int	test_itoa_malloc_fail()
{
	int		fails = 0;
	char	*res;

	printf("[HARDCORE] ft_itoa malloc fails (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	set_malloc_fail_after(0);
	res = ft_itoa(42);
	set_malloc_fail_after(-1);
	if (res == NULL)
		PASS();
	else
	{
		free(res);
		FAIL();
		fails++;
	}
	return (fails);
}

/* ------------------------------------------------------------------ */
/* FT_STRDUP HARDCORE                                                 */
/* ------------------------------------------------------------------ */

static int	test_strdup_malloc_fail()
{
	int		fails = 0;
	char	*res;

	printf("[HARDCORE] ft_strdup malloc fails (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	set_malloc_fail_after(0);
	res = ft_strdup("hello");
	set_malloc_fail_after(-1);
	if (res == NULL)
		PASS();
	else
	{
		free(res);
		FAIL();
		fails++;
	}
	return (fails);
}

/* ------------------------------------------------------------------ */
/* FT_CALLOC HARDCORE                                                 */
/* ------------------------------------------------------------------ */

static int	test_calloc_malloc_fail()
{
	int		fails = 0;
	void	*res;

	printf("[HARDCORE] ft_calloc malloc fails (no crash)\n");
	printf("\e[0;31m -- If the program crashes, you fail! --\n\n \e[0m");
	fflush(stdout);
	set_malloc_fail_after(0);
	res = ft_calloc(10, sizeof(int));
	set_malloc_fail_after(-1);
	if (res == NULL)
		PASS();
	else
	{
		free(res);
		FAIL();
		fails++;
	}
	return (fails);
}

static int	test_calloc_zero()
{
	int		fails = 0;
	void	*res;

	printf("[HARDCORE] ft_calloc(0, 0) returns freeable pointer\n");
	set_malloc_fail_after(-1);
	res = ft_calloc(0, 0);
	if (res != NULL)
	{
		free(res);
		PASS();
	}
	else
	{
		FAIL();
		fails++;
	}
	return (fails);
}

static int	test_calloc_zeroed()
{
	int		fails = 0;
	char	*res;
	int		i;
	int		ok;

	printf("[HARDCORE] ft_calloc memory is zeroed\n");
	set_malloc_fail_after(-1);
	res = ft_calloc(100, sizeof(char));
	if (!res)
	{
		FAIL();
		return (++fails);
	}
	ok = 1;
	i = 0;
	while (i < 100)
	{
		if (res[i] != 0)
			ok = 0;
		i++;
	}
	free(res);
	if (ok)
		PASS();
	else
	{
		FAIL();
		fails++;
	}
	return (fails);
}

/* ------------------------------------------------------------------ */
/* ENTRY POINT                                                        */
/* ------------------------------------------------------------------ */

int	testing_ft_hardcore()
{
	int	fails = 0;

	TITLE("HARDCORE: FT_SPLIT");
	sleep(1);
	fails += test_split_normal();
	sleep(1);
	fails += test_split_malloc_fail_at_0();
	sleep(1);
	fails += test_split_malloc_fail_after_array();
	sleep(1);
	fails += test_split_malloc_fail_mid_words();
	sleep(1);
	fails += test_split_empty_string();
	sleep(1);
	fails += test_split_only_delimiters();
	sleep(1);
	fails += test_split_no_delimiter();
	sleep(1);
	fails += test_split_consecutive_delimiters();
	sleep(1);

	TITLE("HARDCORE: FT_SUBSTR / FT_STRJOIN / FT_STRTRIM / FT_STRMAPI");
	sleep(1);
	fails += test_substr_malloc_fail();
	sleep(1);
	fails += test_strjoin_malloc_fail();
	sleep(1);
	fails += test_strtrim_malloc_fail();
	sleep(1);
	fails += test_strmapi_malloc_fail();
	sleep(1);

	TITLE("HARDCORE: FT_ITOA / FT_STRDUP / FT_CALLOC");
	sleep(1);
	fails += test_itoa_malloc_fail();
	sleep(1);
	fails += test_strdup_malloc_fail();
	sleep(1);
	fails += test_calloc_malloc_fail();
	sleep(1);
	fails += test_calloc_zero();
	sleep(1);
	fails += test_calloc_zeroed();
	sleep(1);

	printf("\n===== Hardcore Testing Complete =====\n");
	return (fails);
}
