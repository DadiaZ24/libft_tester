#include "tester.h"

/*
** PART 2 - Additional functions
** Every returned string must be the start of a live malloc'd block that is
** big enough (t_check_str): returning "" as a literal, s1 itself, or a
** pointer into the input is a KO because the caller will free() it.
*/

/* ------------------------------------------------------------------ */
/* ft_substr                                                          */
/* ------------------------------------------------------------------ */

typedef struct s_sub
{
	const char		*s;
	unsigned int	start;
	size_t			len;
	const char		*exp;
}	t_sub;

static const t_sub	g_subs[] = {
	{"Hello World", 6, 5, "World"}, {"Hello World", 0, 5, "Hello"},
	{"Hello World", 0, 0, ""}, {"Hello World", 0, 11, "Hello World"},
	{"Hello World", 0, 100, "Hello World"}, {"Hello World", 10, 1, "d"},
	{"Hello World", 10, 100, "d"}, {"Hello World", 11, 5, ""},
	{"Hello World", 12, 5, ""}, {"Hello World", 100, 5, ""},
	{"Hello World", UINT_MAX, 5, ""}, {"Hello World", 3, 0, ""},
	{"", 0, 0, ""}, {"", 0, 10, ""}, {"", 1, 1, ""}, {"abc", 1, 1, "b"},
	{"hola", 0, SIZE_MAX, "hola"}, {"hola", 2, SIZE_MAX, "la"},
	{"hola", 2, SIZE_MAX - 1, "la"}, {"hola", 1, SIZE_MAX - 1, "ola"},
	{"hola", 4, SIZE_MAX, ""}, {"hola", 3, SIZE_MAX - 2, "a"},
	{"\xff\x80\x01", 1, 1, "\x80"},
};

static void	test_substr(void)
{
	size_t	i;

	i = 0;
	while (i < sizeof(g_subs) / sizeof(*g_subs))
	{
		CASE("ft_substr(\"%s\", %u, %zu) [guarded]", t_esc(g_subs[i].s), g_subs[i].start, g_subs[i].len);
		t_check_str("ft_substr", ft_substr(t_gstr(g_subs[i].s), g_subs[i].start, g_subs[i].len), g_subs[i].exp);
		CASE("ft_substr(\"%s\", %u, %zu) [front guarded]", t_esc(g_subs[i].s), g_subs[i].start, g_subs[i].len);
		t_check_str("ft_substr", ft_substr(t_gstr_front(g_subs[i].s), g_subs[i].start, g_subs[i].len), g_subs[i].exp);
		i++;
	}
}

static void	test_substr_alloc_size(void)
{
	char	*r;

	CASE("ft_substr(\"hola\", 0, 1000000)");
	r = ft_substr("hola", 0, 1000000);
	EXPECT(!r || t_block_size(r) <= 5, "allocated %zu bytes for \"hola\" (5 are enough)", t_block_size(r));
	t_free(r);
	CASE("ft_substr(\"hola\", 1, 3)");
	r = ft_substr("hola", 1, 3);
	EXPECT(!r || t_block_size(r) <= 4, "allocated %zu bytes for \"ola\" (4 are enough)", t_block_size(r));
	t_free(r);
	CASE("ft_substr(\"hola\", 10, 42)");
	r = ft_substr("hola", 10, 42);
	EXPECT(!r || t_block_size(r) <= 1, "allocated %zu bytes for \"\" (1 is enough)", t_block_size(r));
	t_free(r);
}

static void	test_substr_null(void)
{
	char	*r;

	CASE("ft_substr(NULL, 0, 5)");
	r = ft_substr(NULL, 0, 5);
	EXPECT(r == NULL, "should return NULL");
	t_free(r);
}

/* ------------------------------------------------------------------ */
/* ft_strjoin                                                         */
/* ------------------------------------------------------------------ */

static void	test_strjoin(void)
{
	const char	*p[][3] = {{"Hello", " World", "Hello World"}, {"", "World", "World"},
		{"Hello", "", "Hello"}, {"", "", ""}, {"\xff", "\x80", "\xff\x80"},
		{"42", "school", "42school"}, {"a", "b", "ab"}};
	size_t		i;
	size_t		n;
	char		*a;
	char		*b;
	char		*r;

	i = 0;
	while (i < sizeof(p) / sizeof(*p))
	{
		CASE("ft_strjoin(\"%s\", \"%s\") [guarded]", t_esc(p[i][0]), t_esc(p[i][1]));
		t_check_str("ft_strjoin", ft_strjoin(t_gstr(p[i][0]), t_gstr(p[i][1])), p[i][2]);
		CASE("ft_strjoin(\"%s\", \"%s\") [front guarded]", t_esc(p[i][0]), t_esc(p[i][1]));
		t_check_str("ft_strjoin", ft_strjoin(t_gstr_front(p[i][0]), t_gstr_front(p[i][1])), p[i][2]);
		i++;
	}
	n = 2 << 20;
	a = t_gmem(NULL, n + 1);
	b = t_gmem(NULL, n + 1);
	memset(a, 'a', n);
	memset(b, 'b', n);
	a[n] = 0;
	b[n] = 0;
	CASE("ft_strjoin(<2 MB>, <2 MB>) [guarded]");
	r = ft_strjoin(a, b);
	EXPECT(r && t_block_size(r) >= 2 * n + 1 && r[0] == 'a' && r[n - 1] == 'a'
		&& r[n] == 'b' && r[2 * n - 1] == 'b' && r[2 * n] == 0, "wrong result");
	t_free(r);
}

static void	test_strjoin_null(void)
{
	CASE("ft_strjoin(NULL, \"abc\")");
	t_free(ft_strjoin(NULL, "abc"));
	CASE("ft_strjoin(\"abc\", NULL)");
	t_free(ft_strjoin("abc", NULL));
	CASE("ft_strjoin(NULL, NULL)");
	t_free(ft_strjoin(NULL, NULL));
}

/* ------------------------------------------------------------------ */
/* ft_strtrim                                                         */
/* ------------------------------------------------------------------ */

static void	test_strtrim(void)
{
	const char	*p[][3] = {
		{"  Hello World  ", " ", "Hello World"}, {"xxhixx", "x", "hi"}, {"hi", "", "hi"},
		{"", "abc", ""}, {"", "", ""}, {"aaaa", "a", ""}, {"a", "a", ""}, {"abcba", "ab", "c"},
		{"a b a", " a", "b"}, {"\t\n hello \n\t", " \t\n", "hello"}, {"   xxx   xxx", " x", ""},
		{"lorem ipsum dolor sit amet", "te", "lorem ipsum dolor sit am"},
		{"lorem \n ipsum \t dolor \n sit \t amet", " ", "lorem \n ipsum \t dolor \n sit \t amet"},
		{"lorem ipsum dolor sit amet", "tel", "orem ipsum dolor sit am"},
		{"  \t \t \n   \n\n\n\t", " \n\t", ""}, {"abc", "xyz", "abc"}, {"x", "xyz", ""},
		{"\xc8hello\xc8", "\xc8", "hello"}, {"abcdba", "acb", "d"}};
	size_t		i;
	char		*s;
	char		*r;

	i = 0;
	while (i < sizeof(p) / sizeof(*p))
	{
		CASE("ft_strtrim(\"%s\", \"%s\") [guarded]", t_esc(p[i][0]), t_esc(p[i][1]));
		s = t_gstr(p[i][0]);
		r = ft_strtrim(s, t_gstr(p[i][1]));
		EXPECT(r != s, "must return a new string, not s1");
		t_check_str("ft_strtrim", r, p[i][2]);
		CASE("ft_strtrim(\"%s\", \"%s\") [front guarded]", t_esc(p[i][0]), t_esc(p[i][1]));
		t_check_str("ft_strtrim", ft_strtrim(t_gstr_front(p[i][0]), t_gstr_front(p[i][1])), p[i][2]);
		i++;
	}
}

static void	test_strtrim_null(void)
{
	CASE("ft_strtrim(NULL, \" \")");
	t_free(ft_strtrim(NULL, " "));
	CASE("ft_strtrim(\"  abc  \", NULL)");
	t_free(ft_strtrim("  abc  ", NULL));
}

/* ------------------------------------------------------------------ */
/* ft_split                                                           */
/* ------------------------------------------------------------------ */

typedef struct s_spl
{
	const char	*s;
	char		c;
	const char	*exp[16];
}	t_spl;

static const t_spl	g_spls[] = {
	{"a,b,c", ',', {"a", "b", "c", NULL}},
	{"", ',', {NULL}},
	{",,,", ',', {NULL}},
	{"abc", ',', {"abc", NULL}},
	{",a,,b,", ',', {"a", "b", NULL}},
	{"  hello   world  ", ' ', {"hello", "world", NULL}},
	{"abc", '\0', {"abc", NULL}},
	{"", '\0', {NULL}},
	{"a\xc8" "b\xc8\xc8" "c", (char)0xc8, {"a", "b", "c", NULL}},
	{"x", 'x', {NULL}},
	{"xax", 'x', {"a", NULL}},
	{"a", 'x', {"a", NULL}},
	{"      split       this for   me  !       ", ' ', {"split", "this", "for", "me", "!", NULL}},
	{"\t\t\t\thello!\t\t\t\t", '\t', {"hello!", NULL}},
	{"0 0 0 0 0 0 0 0 0", ' ', {"0", "0", "0", "0", "0", "0", "0", "0", "0", NULL}},
	{"lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed non risus.", ' ',
		{"lorem", "ipsum", "dolor", "sit", "amet,", "consectetur", "adipiscing", "elit.",
		"Sed", "non", "risus.", NULL}},
	{"^^^1^^2a,^^^^3^^^^--h^^^^", '^', {"1", "2a,", "3", "--h", NULL}},
	{"nonempty", ' ', {"nonempty", NULL}},
};

static void	check_split(char **r, const char *const *exp)
{
	size_t	n;
	size_t	i;

	n = 0;
	while (exp[n])
		n++;
	if (!r)
	{
		t_fail("returned NULL");
		return ;
	}
	if (!t_is_block(r))
	{
		t_fail("the array is not a malloc'd block");
		return ;
	}
	if (t_block_size(r) < (n + 1) * sizeof(char *))
	{
		t_fail("array allocated with %zu bytes, needs %zu for %zu word(s) + NULL",
			t_block_size(r), (n + 1) * sizeof(char *), n);
		t_free_split(r);
		return ;
	}
	i = 0;
	while (i < n)
	{
		if (!r[i])
		{
			t_fail("only %zu word(s), expected %zu", i, n);
			break ;
		}
		if (!t_is_block(r[i]))
		{
			t_fail("word %zu is not a malloc'd block", i);
			break ;
		}
		if (t_block_size(r[i]) < strlen(exp[i]) + 1 || memcmp(r[i], exp[i], strlen(exp[i]) + 1))
		{
			t_fail("word %zu: expected \"%s\", got \"%s\"", i, t_esc(exp[i]),
				t_escn(r[i], t_block_size(r[i]) < strlen(exp[i]) + 1 ? t_block_size(r[i]) : strlen(exp[i]) + 1));
			break ;
		}
		i++;
	}
	if (i == n)
		EXPECT(r[n] == NULL, "the array is not NULL-terminated after %zu word(s)", n);
	t_free_split(r);
}

static void	test_split(void)
{
	size_t	i;

	i = 0;
	while (i < sizeof(g_spls) / sizeof(*g_spls))
	{
		CASE("ft_split(\"%s\", %d) [guarded]", t_esc(g_spls[i].s), g_spls[i].c);
		check_split(ft_split(t_gstr(g_spls[i].s), g_spls[i].c), g_spls[i].exp);
		CASE("ft_split(\"%s\", %d) [front guarded]", t_esc(g_spls[i].s), g_spls[i].c);
		check_split(ft_split(t_gstr_front(g_spls[i].s), g_spls[i].c), g_spls[i].exp);
		i++;
	}
}

static void	test_split_big(void)
{
	size_t	words;
	size_t	n;
	size_t	i;
	char	*s;
	char	**r;

	words = 100000;
	n = words * 3;
	s = t_gmem(NULL, n + 1);
	i = 0;
	while (i < words)
	{
		s[i * 3] = 'a' + i % 26;
		s[i * 3 + 1] = 'z' - i % 26;
		s[i * 3 + 2] = ' ';
		i++;
	}
	s[n] = 0;
	CASE("ft_split(<100 000 words>, ' ') [guarded]");
	r = ft_split(s, ' ');
	if (!r)
	{
		t_fail("returned NULL");
		return ;
	}
	i = 0;
	while (i < words && r[i] && r[i][0] == 'a' + (char)(i % 26) && r[i][1] == 'z' - (char)(i % 26) && !r[i][2])
		i++;
	EXPECT(i == words && r[words] == NULL, "wrong word %zu", i);
	t_free_split(r);
	n = 4 << 20;
	s = t_gmem(NULL, n + 1);
	memset(s, 'w', n);
	s[n] = 0;
	CASE("ft_split(<4 MB without delimiter>, ' ') [guarded]");
	r = ft_split(s, ' ');
	EXPECT(r && r[0] && t_block_size(r[0]) >= n + 1 && r[0][n - 1] == 'w' && r[0][n] == 0 && !r[1],
		"wrong result");
	t_free_split(r);
}

static void	test_split_null(void)
{
	char	**r;

	CASE("ft_split(NULL, ' ')");
	r = ft_split(NULL, ' ');
	EXPECT(r == NULL, "should return NULL");
	t_free_split(r);
}

/* ------------------------------------------------------------------ */
/* ft_itoa                                                            */
/* ------------------------------------------------------------------ */

static void	check_itoa(int n)
{
	char	exp[16];

	snprintf(exp, sizeof(exp), "%d", n);
	CASE("ft_itoa(%d)", n);
	t_check_str("ft_itoa", ft_itoa(n), exp);
}

static void	test_itoa(void)
{
	int				vals[] = {0, 1, -1, 9, -9, 10, -10, 42, -42, 99, 100, -100, 101, 12345,
		-12345, 1000000000, -1000000000, 999999999, INT_MAX, INT_MIN, INT_MAX - 1, INT_MIN + 1};
	size_t			i;
	unsigned int	x;

	i = 0;
	while (i < sizeof(vals) / sizeof(*vals))
		check_itoa(vals[i++]);
	x = 42;
	i = 0;
	while (i < 20000)
	{
		x = x * 1103515245u + 12345u;
		check_itoa((int)x >> (i % 31));
		i++;
	}
}

/* ------------------------------------------------------------------ */
/* ft_strmapi / ft_striteri                                           */
/* ------------------------------------------------------------------ */

static unsigned int	g_idx[64];
static char			*g_ptrs[64];
static int			g_ncalls;

static char	map_rec(unsigned int i, char c)
{
	if (g_ncalls < 64)
		g_idx[g_ncalls] = i;
	g_ncalls++;
	return (c + 1);
}

static void	iter_rec(unsigned int i, char *c)
{
	if (g_ncalls < 64)
	{
		g_idx[g_ncalls] = i;
		g_ptrs[g_ncalls] = c;
	}
	g_ncalls++;
	*c = ft_toupper(*c);
}

static void	test_strmapi(void)
{
	char	*s;
	char	*r;
	int		ok;
	size_t	n;

	g_ncalls = 0;
	CASE("ft_strmapi(\"abc\", f) [guarded]");
	t_check_str("ft_strmapi", ft_strmapi(t_gstr("abc"), map_rec), "bcd");
	EXPECT(g_ncalls == 3 && g_idx[0] == 0 && g_idx[1] == 1 && g_idx[2] == 2,
		"f must be called once per char with index 0, 1, 2 (called %d time(s))", g_ncalls);
	g_ncalls = 0;
	CASE("ft_strmapi(\"\", f) [guarded]");
	t_check_str("ft_strmapi", ft_strmapi(t_gstr(""), map_rec), "");
	EXPECT(g_ncalls == 0, "f must not be called for \"\"");
	CASE("ft_strmapi(\"hello\", f) [front guarded]");
	t_check_str("ft_strmapi", ft_strmapi(t_gstr_front("hello"), map_rec), "ifmmp");
	n = 1 << 20;
	s = t_gmem(NULL, n + 1);
	memset(s, 'a', n);
	s[n] = 0;
	g_ncalls = 0;
	CASE("ft_strmapi(<1 MB>, f) [guarded]");
	r = ft_strmapi(s, map_rec);
	ok = r && t_block_size(r) >= n + 1 && r[0] == 'b' && r[n - 1] == 'b' && r[n] == 0;
	EXPECT(ok && g_ncalls == (int)n, "wrong result");
	t_free(r);
}

static void	test_striteri(void)
{
	char	*s;
	int		i;
	int		ok;

	s = t_gstr("hello");
	g_ncalls = 0;
	CASE("ft_striteri(\"hello\", f) [guarded]");
	ft_striteri(s, iter_rec);
	EXPECT(strcmp(s, "HELLO") == 0, "expected \"HELLO\", got \"%s\"", t_esc(s));
	ok = g_ncalls == 5;
	i = 0;
	while (ok && i < 5)
	{
		ok = (g_idx[i] == (unsigned)i && g_ptrs[i] == s + i);
		i++;
	}
	EXPECT(ok, "f must get (i, &s[i]) for each char, in order (called %d time(s))", g_ncalls);
	g_ncalls = 0;
	CASE("ft_striteri(\"\", f) [guarded]");
	ft_striteri(t_gstr(""), iter_rec);
	EXPECT(g_ncalls == 0, "f must not be called for \"\"");
	CASE("ft_striteri(\"abc\", f) [front guarded]");
	s = t_gstr_front("abc");
	ft_striteri(s, iter_rec);
	EXPECT(strcmp(s, "ABC") == 0, "expected \"ABC\", got \"%s\"", t_esc(s));
}

static void	test_mapi_iteri_null(void)
{
	CASE("ft_strmapi(NULL, f)");
	t_free(ft_strmapi(NULL, map_rec));
	CASE("ft_strmapi(\"abc\", NULL)");
	t_free(ft_strmapi("abc", NULL));
	CASE("ft_striteri(NULL, f)");
	ft_striteri(NULL, iter_rec);
	CASE("ft_striteri(\"abc\", NULL)");
	ft_striteri(t_gstr("abc"), NULL);
}

/* ------------------------------------------------------------------ */
/* ft_put*_fd                                                         */
/* ------------------------------------------------------------------ */

static void	test_putchar_fd(void)
{
	int		fd;
	char	buf[16];
	size_t	n;
	char	cs[] = {'a', '\0', (char)200, '\n', (char)255};
	size_t	i;

	fd = t_capture_open();
	i = 0;
	while (i < sizeof(cs))
	{
		CASE("ft_putchar_fd(%d, fd)", cs[i]);
		ft_putchar_fd(cs[i], fd);
		n = t_capture_read(fd, buf, sizeof(buf));
		EXPECT(n == 1 && buf[0] == cs[i], "expected exactly 1 byte (0x%02x), got %zu byte(s)",
			(unsigned char)cs[i], n);
		i++;
	}
	close(fd);
}

static void	test_putstr_fd(void)
{
	int		fd;
	char	*buf;
	char	*big;
	size_t	n;
	size_t	sz;

	fd = t_capture_open();
	buf = t_gmem(NULL, 300000);
	CASE("ft_putstr_fd(\"hello\", fd) [guarded]");
	ft_putstr_fd(t_gstr("hello"), fd);
	n = t_capture_read(fd, buf, 300000);
	EXPECT(n == 5 && !memcmp(buf, "hello", 5), "got \"%s\"", t_escn(buf, n));
	CASE("ft_putstr_fd(\"\", fd)");
	ft_putstr_fd(t_gstr(""), fd);
	EXPECT(t_capture_read(fd, buf, 300000) == 0, "wrote something for \"\"");
	CASE("ft_putstr_fd(\"\\xff\\x80 ok\", fd)");
	ft_putstr_fd(t_gstr("\xff\x80 ok"), fd);
	n = t_capture_read(fd, buf, 300000);
	EXPECT(n == 5 && !memcmp(buf, "\xff\x80 ok", 5), "got \"%s\"", t_escn(buf, n));
	sz = 200000;
	big = t_gmem(NULL, sz + 1);
	memset(big, 'q', sz);
	big[sz] = 0;
	CASE("ft_putstr_fd(<200 KB>, fd) [guarded]");
	ft_putstr_fd(big, fd);
	n = t_capture_read(fd, buf, 300000);
	EXPECT(n == sz && buf[0] == 'q' && buf[sz - 1] == 'q', "wrote %zu byte(s), expected %zu", n, sz);
	close(fd);
}

static void	test_putendl_fd(void)
{
	int		fd;
	char	buf[64];
	size_t	n;

	fd = t_capture_open();
	CASE("ft_putendl_fd(\"hello\", fd) [guarded]");
	ft_putendl_fd(t_gstr("hello"), fd);
	n = t_capture_read(fd, buf, sizeof(buf));
	EXPECT(n == 6 && !memcmp(buf, "hello\n", 6), "got \"%s\"", t_escn(buf, n));
	CASE("ft_putendl_fd(\"\", fd)");
	ft_putendl_fd(t_gstr(""), fd);
	n = t_capture_read(fd, buf, sizeof(buf));
	EXPECT(n == 1 && buf[0] == '\n', "expected \"\\n\", got \"%s\"", t_escn(buf, n));
	close(fd);
}

static void	test_putnbr_fd(void)
{
	int				vals[] = {0, 1, -1, 9, -9, 10, -10, 42, -42, 100, -100, 12345, INT_MAX,
		INT_MIN, INT_MAX - 1, INT_MIN + 1, 1000000000, -1000000000};
	int				fd;
	char			buf[64];
	char			exp[16];
	size_t			n;
	size_t			i;
	unsigned int	x;
	int				v;

	fd = t_capture_open();
	x = 7;
	i = 0;
	while (i < sizeof(vals) / sizeof(*vals) + 2000)
	{
		if (i < sizeof(vals) / sizeof(*vals))
			v = vals[i];
		else
		{
			x = x * 1103515245u + 12345u;
			v = (int)x >> (i % 31);
		}
		snprintf(exp, sizeof(exp), "%d", v);
		CASE("ft_putnbr_fd(%d, fd)", v);
		ft_putnbr_fd(v, fd);
		n = t_capture_read(fd, buf, sizeof(buf));
		EXPECT(n == strlen(exp) && !memcmp(buf, exp, n), "expected \"%s\", got \"%s\"", exp, t_escn(buf, n));
		i++;
	}
	close(fd);
}

static void	test_put_bad_fd(void)
{
	CASE("ft_putchar_fd('a', -1)");
	ft_putchar_fd('a', -1);
	CASE("ft_putstr_fd(\"abc\", -1)");
	ft_putstr_fd("abc", -1);
	CASE("ft_putendl_fd(\"abc\", -1)");
	ft_putendl_fd("abc", -1);
	CASE("ft_putnbr_fd(INT_MIN, -1)");
	ft_putnbr_fd(INT_MIN, -1);
	CASE("ft_putnbr_fd(42, 4242) (fd not open)");
	ft_putnbr_fd(42, 4242);
}

static void	test_put_null(void)
{
	CASE("ft_putstr_fd(NULL, fd)");
	ft_putstr_fd(NULL, -1);
	CASE("ft_putendl_fd(NULL, fd)");
	ft_putendl_fd(NULL, -1);
}

/* ------------------------------------------------------------------ */

void	run_part2(void)
{
	t_section("PART 2 - ADDITIONAL FUNCTIONS");
	t_run("ft_substr (incl. start > len, len = SIZE_MAX)", test_substr, T_MUST, 5);
	t_run("ft_substr does not over-allocate", test_substr_alloc_size, T_WARN, 5);
	t_run("ft_substr(NULL, ...) (UB)", test_substr_null, T_WARN, 5);
	t_run("ft_strjoin", test_strjoin, T_MUST, 5);
	t_run("ft_strjoin with NULL (UB)", test_strjoin_null, T_WARN, 5);
	t_run("ft_strtrim (incl. everything trimmed)", test_strtrim, T_MUST, 5);
	t_run("ft_strtrim with NULL (UB)", test_strtrim_null, T_WARN, 5);
	t_run("ft_split", test_split, T_MUST, 5);
	t_run("ft_split big inputs (100k words / 4 MB word)", test_split_big, T_MUST, 10);
	t_run("ft_split(NULL, c) (UB)", test_split_null, T_WARN, 5);
	t_run("ft_itoa (incl. INT_MIN + 20000 random values)", test_itoa, T_MUST, 10);
	t_run("ft_strmapi", test_strmapi, T_MUST, 5);
	t_run("ft_striteri", test_striteri, T_MUST, 5);
	t_run("ft_strmapi / ft_striteri with NULL (UB)", test_mapi_iteri_null, T_WARN, 5);
	t_run("ft_putchar_fd", test_putchar_fd, T_MUST, 5);
	t_run("ft_putstr_fd", test_putstr_fd, T_MUST, 5);
	t_run("ft_putendl_fd", test_putendl_fd, T_MUST, 5);
	t_run("ft_putnbr_fd (incl. INT_MIN)", test_putnbr_fd, T_MUST, 10);
	t_run("ft_put*_fd on an invalid fd must not crash", test_put_bad_fd, T_MUST, 5);
	t_run("ft_putstr_fd / ft_putendl_fd with NULL (UB)", test_put_null, T_WARN, 5);
}
