#include "tester.h"
#include <stddef.h>

/*
** Prototypes from subject v19.3 (libc prototypes without 'restrict').
** Compared with the prototypes of your libft.h (renamed user_ft_*, see
** libft_api.h) at compile time, reported at run time: a mismatch or a
** function not declared yet never stops the build.
*/

typedef struct s_proto
{
	const char	*name;
	const char	*expected;
	int			ok;
}	t_proto;

/* CHK_ft_x (probe.h): 1 = same type, 0 = different, -1 = not declared */
#define P(fn, type) {#fn, #type, CHK_##fn(type)}

static const t_proto	g_protos[] = {
	P(ft_isalpha, int (*)(int)),
	P(ft_isdigit, int (*)(int)),
	P(ft_isalnum, int (*)(int)),
	P(ft_isascii, int (*)(int)),
	P(ft_isprint, int (*)(int)),
	P(ft_strlen, size_t (*)(const char *)),
	P(ft_memset, void *(*)(void *, int, size_t)),
	P(ft_bzero, void (*)(void *, size_t)),
	P(ft_memcpy, void *(*)(void *, const void *, size_t)),
	P(ft_memmove, void *(*)(void *, const void *, size_t)),
	P(ft_strlcpy, size_t (*)(char *, const char *, size_t)),
	P(ft_strlcat, size_t (*)(char *, const char *, size_t)),
	P(ft_toupper, int (*)(int)),
	P(ft_tolower, int (*)(int)),
	P(ft_strchr, char *(*)(const char *, int)),
	P(ft_strrchr, char *(*)(const char *, int)),
	P(ft_strncmp, int (*)(const char *, const char *, size_t)),
	P(ft_memchr, void *(*)(const void *, int, size_t)),
	P(ft_memcmp, int (*)(const void *, const void *, size_t)),
	P(ft_strnstr, char *(*)(const char *, const char *, size_t)),
	P(ft_atoi, int (*)(const char *)),
	P(ft_calloc, void *(*)(size_t, size_t)),
	P(ft_strdup, char *(*)(const char *)),
	P(ft_substr, char *(*)(char const *, unsigned int, size_t)),
	P(ft_strjoin, char *(*)(char const *, char const *)),
	P(ft_strtrim, char *(*)(char const *, char const *)),
	P(ft_split, char **(*)(char const *, char)),
	P(ft_itoa, char *(*)(int)),
	P(ft_strmapi, char *(*)(char const *, char (*)(unsigned int, char))),
	P(ft_striteri, void (*)(char *, void (*)(unsigned int, char *))),
	P(ft_putchar_fd, void (*)(char, int)),
	P(ft_putstr_fd, void (*)(char *, int)),
	P(ft_putendl_fd, void (*)(char *, int)),
	P(ft_putnbr_fd, void (*)(int, int)),
	P(ft_lstnew, t_list *(*)(void *)),
	P(ft_lstadd_front, void (*)(t_list **, t_list *)),
	P(ft_lstsize, unsigned int (*)(t_list *)),
	P(ft_lstlast, t_list *(*)(t_list *)),
	P(ft_lstadd_back, void (*)(t_list **, t_list *)),
	P(ft_lstdelone, void (*)(t_list *, void (*)(void *))),
	P(ft_lstclear, void (*)(t_list **, void (*)(void *))),
	P(ft_lstiter, void (*)(t_list *, void (*)(void *))),
	P(ft_lstmap, t_list *(*)(t_list *, void *(*)(void *), void (*)(void *))),
};

static void	check_protos(size_t from, size_t to)
{
	size_t	i;

	i = from;
	while (i < to && i < sizeof(g_protos) / sizeof(*g_protos))
	{
		CASE("%s", g_protos[i].name);
		if (g_protos[i].ok < 0)
			t_fail("not declared in libft.h, expected type %s", g_protos[i].expected);
		else
			EXPECT(g_protos[i].ok, "prototype differs from the subject, expected type %s", g_protos[i].expected);
		i++;
	}
}

static void	protos_part1(void)
{
	check_protos(0, 23);
}

static void	protos_part2(void)
{
	check_protos(23, 34);
}

static void	protos_list(void)
{
	check_protos(34, 43);
}

static void	test_t_list(void)
{
#if HAS_T_LIST
	t_list	l;

	CASE("typedef struct s_list { void *content; struct s_list *next; } t_list;");
	EXPECT(offsetof(t_list, content) == 0 && offsetof(t_list, next) == sizeof(void *)
		&& sizeof(t_list) == 2 * sizeof(void *), "t_list is not exactly the struct of the subject");
	EXPECT(__builtin_types_compatible_p(__typeof__(l.content), void *), "content must be void *");
	EXPECT(__builtin_types_compatible_p(__typeof__(l.next), struct s_list *), "next must be struct s_list *");
#else
	t_fail("t_list is not defined in libft.h (the tester uses the subject's struct meanwhile)");
#endif
}

static const t_test	g_protos_tests[] = {
	TEST("prototypes of part 1 (23 functions)", "Each prototype of libft.h must be the libc one, without \
restrict: const char * vs char *, int c vs char c, size_t vs int... A different type is a KO in \
evaluation (and can break the callers).", protos_part1),
	TEST("prototypes of part 2 (11 functions)", "Exactly the prototypes of the subject (char const *, \
unsigned int start, the function pointers of strmapi / striteri...).", protos_part2),
	TEST("prototypes of the list functions (9)", "Exactly the prototypes of the subject v19.3: note that \
ft_lstsize returns unsigned int in this version.", protos_list),
	TEST("t_list struct", "typedef struct s_list { void *content; struct s_list *next; } t_list; exactly, \
in libft.h.", test_t_list),
};

void	run_prototypes(void)
{
	t_section("PROTOTYPES (subject v19.3)");
	GROUP_OTHER("libft.h", g_protos_tests);
}
