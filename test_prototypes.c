#include "tester.h"
#include <stddef.h>

/*
** Prototypes from subject v19.3 (libc prototypes without 'restrict').
** Checked at compile time with __builtin_types_compatible_p, reported at
** run time so a single mismatch doesn't stop the whole tester.
*/

typedef struct s_proto
{
	const char	*name;
	const char	*expected;
	int			ok;
}	t_proto;

#define P(fn, type) {#fn, #type, __builtin_types_compatible_p(__typeof__(&fn), type)}

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

static void	test_prototypes(void)
{
	size_t	i;

	i = 0;
	while (i < sizeof(g_protos) / sizeof(*g_protos))
	{
		CASE("%s", g_protos[i].name);
		EXPECT(g_protos[i].ok, "prototype differs from the subject, expected type %s", g_protos[i].expected);
		i++;
	}
}

static void	test_t_list(void)
{
	t_list	l;

	CASE("typedef struct s_list { void *content; struct s_list *next; } t_list;");
	EXPECT(offsetof(t_list, content) == 0 && offsetof(t_list, next) == sizeof(void *)
		&& sizeof(t_list) == 2 * sizeof(void *), "t_list is not exactly the struct of the subject");
	EXPECT(__builtin_types_compatible_p(__typeof__(l.content), void *), "content must be void *");
	EXPECT(__builtin_types_compatible_p(__typeof__(l.next), struct s_list *), "next must be struct s_list *");
}

void	run_prototypes(void)
{
	t_section("PROTOTYPES (subject v19.3)");
	t_run("prototypes of all 43 functions (ft_lstsize returns unsigned int!)", test_prototypes, T_MUST, 5);
	t_run("t_list struct", test_t_list, T_MUST, 5);
}
