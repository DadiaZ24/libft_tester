#include "tester.h"

/*
** PART 2 - Additional functions
** Every returned string must be the start of a live malloc'd block that is
** big enough (t_check_str): returning "" as a literal, s1 itself, or a
** pointer into the input is a KO because the caller will free() it.
*/

static char	*big_gstr(size_t n, char c)
{
	char	*s;

	s = t_gmem(NULL, n + 1);
	memset(s, c, n);
	s[n] = 0;
	return (s);
}

/* ================================================================== */
/* ft_substr                                                          */
/* ================================================================== */

static void	sub_one(const char *s, unsigned int start, size_t len, const char *exp)
{
	CASE("ft_substr(\"%s\", %u, %zu) [guarded]", t_esc(s), start, len);
	t_check_str("", ft_substr(t_gstr(s), start, len), exp);
	CASE("ft_substr(\"%s\", %u, %zu) [front guarded]", t_esc(s), start, len);
	t_check_str("", ft_substr(t_gstr_front(s), start, len), exp);
}

static void	sub_basic(void)
{
	sub_one("Hello World", 6, 5, "World");
	sub_one("Hello World", 0, 5, "Hello");
	sub_one("Hello World", 0, 11, "Hello World");
	sub_one("Hello World", 10, 1, "d");
	sub_one("abc", 1, 1, "b");
	sub_one("\xff\x80\x01", 1, 1, "\x80");
}

static void	sub_start_out(void)
{
	sub_one("Hello World", 11, 5, "");
	sub_one("Hello World", 12, 5, "");
	sub_one("Hello World", 100, 5, "");
	sub_one("Hello World", UINT_MAX, 5, "");
	sub_one("", 1, 1, "");
	sub_one("hola", 4, 1, "");
}

static void	sub_len_long(void)
{
	sub_one("Hello World", 0, 100, "Hello World");
	sub_one("Hello World", 10, 100, "d");
	sub_one("Hello World", 6, 6, "World");
	sub_one("", 0, 10, "");
	sub_one("abc", 2, 2, "c");
}

static void	sub_len_zero(void)
{
	sub_one("Hello World", 0, 0, "");
	sub_one("Hello World", 3, 0, "");
	sub_one("Hello World", 11, 0, "");
	sub_one("", 0, 0, "");
}

static void	sub_max(void)
{
	sub_one("hola", 0, SIZE_MAX, "hola");
	sub_one("hola", 2, SIZE_MAX, "la");
	sub_one("hola", 2, SIZE_MAX - 1, "la");
	sub_one("hola", 1, SIZE_MAX - 1, "ola");
	sub_one("hola", 4, SIZE_MAX, "");
	sub_one("hola", 3, SIZE_MAX - 2, "a");
	sub_one("hola", UINT_MAX, SIZE_MAX, "");
}

static void	sub_alloc(void)
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

static void	sub_big(void)
{
	char	*s;
	char	*r;
	size_t	n;

	n = 8 << 20;
	s = big_gstr(n, 's');
	CASE("ft_substr(<8 MB>, 1, SIZE_MAX) [guarded]");
	r = ft_substr(s, 1, SIZE_MAX);
	EXPECT(r && t_block_size(r) >= n && r[n - 2] == 's' && r[n - 1] == 0, "wrong result");
	t_free(r);
	CASE("ft_substr(<8 MB>, 8 MB - 3, 2) [guarded]");
	t_check_str("", ft_substr(s, n - 3, 2), "ss");
}

static void	mf_substr(void)
{
	CASE("ft_substr(\"Hello World\", 6, 5), a malloc fails");
	MF_STR(ft_substr(t_gstr("Hello World"), 6, 5), "World");
}

static void	mf_substr_empty(void)
{
	CASE("ft_substr(\"hola\", 42, 3), a malloc fails");
	MF_STR(ft_substr(t_gstr("hola"), 42, 3), "");
}

static void	sub_null(void)
{
	char	*r;

	CASE("ft_substr(NULL, 0, 5)");
	r = ft_substr(NULL, 0, 5);
	EXPECT(r == NULL, "should return NULL");
	t_free(r);
}

static const t_test	g_ft_substr[] = {
	TEST("basic substrings", "Returns a new malloc'd string with len chars of s from index start.", sub_basic),
	TEST("start >= strlen(s) gives \"\"", "If start is past the end, the result is a malloc'd \"\" (not \
NULL, not a literal). Computing s + start without a check reads out of the string.", sub_start_out),
	TEST("len longer than the rest", "len is a maximum: only the chars up to the end of s are copied.", sub_len_long),
	TEST("len = 0 gives \"\"", "With len 0 the result is a malloc'd \"\".", sub_len_zero),
	TEST("len = SIZE_MAX, start = UINT_MAX", "start + len overflows, malloc(len + 1) is malloc(0): the \
length of the result must be computed from what is left of s.", sub_max),
	TEST_UB("does not over-allocate", "substr(\"hola\", 0, 1000000) needs 5 bytes, not 1000001: \
evaluators check it with a leak / memory tool.", sub_alloc),
	TEST_SLOW("8 MB string", "A big substring, and a tiny one at the end of a big string.", sub_big),
	TEST_MF("malloc fails", "When its malloc fails, substr returns NULL.", mf_substr),
	TEST_MF("malloc fails, start past the end", "Also for the \"\" returned when start >= strlen(s).", mf_substr_empty),
	TEST_UB("NULL s", "ft_substr(NULL, ...) is not defined by the subject, returning NULL is the usual \
choice: evaluators try it.", sub_null),
};

/* ================================================================== */
/* ft_strjoin                                                         */
/* ================================================================== */

static void	join_one(const char *a, const char *b, const char *exp)
{
	CASE("ft_strjoin(\"%s\", \"%s\") [guarded]", t_esc(a), t_esc(b));
	t_check_str("", ft_strjoin(t_gstr(a), t_gstr(b)), exp);
	CASE("ft_strjoin(\"%s\", \"%s\") [front guarded]", t_esc(a), t_esc(b));
	t_check_str("", ft_strjoin(t_gstr_front(a), t_gstr_front(b)), exp);
}

static void	join_basic(void)
{
	join_one("Hello", " World", "Hello World");
	join_one("42", "school", "42school");
	join_one("a", "b", "ab");
	join_one("lorem ipsum", " dolor sit amet", "lorem ipsum dolor sit amet");
}

static void	join_empty1(void)
{
	join_one("", "World", "World");
	join_one("", "x", "x");
}

static void	join_empty2(void)
{
	join_one("Hello", "", "Hello");
	join_one("x", "", "x");
}

static void	join_empty(void)
{
	join_one("", "", "");
}

static void	join_high(void)
{
	join_one("\xff", "\x80", "\xff\x80");
	join_one("caf\xc3\xa9", " cr\xc3\xa8me", "caf\xc3\xa9 cr\xc3\xa8me");
}

static void	join_same(void)
{
	char	*s;

	s = t_gstr("abc");
	CASE("ft_strjoin(s, s) with s = \"abc\"");
	t_check_str("", ft_strjoin(s, s), "abcabc");
	CASE("ft_strjoin(s, s + 1) with s = \"abc\"");
	t_check_str("", ft_strjoin(s, s + 1), "abcbc");
	EXPECT(!strcmp(s, "abc"), "the inputs were modified");
}

static void	join_big(void)
{
	char	*a;
	char	*b;
	char	*r;
	size_t	n;

	n = 4 << 20;
	a = big_gstr(n, 'a');
	b = big_gstr(n, 'b');
	CASE("ft_strjoin(<4 MB>, <4 MB>) [guarded]");
	r = ft_strjoin(a, b);
	EXPECT(r && t_block_size(r) >= 2 * n + 1 && r[0] == 'a' && r[n - 1] == 'a'
		&& r[n] == 'b' && r[2 * n - 1] == 'b' && r[2 * n] == 0, "wrong result");
	t_free(r);
}

static void	mf_strjoin(void)
{
	CASE("ft_strjoin(\"Hello\", \" World\"), a malloc fails");
	MF_STR(ft_strjoin(t_gstr("Hello"), t_gstr(" World")), "Hello World");
}

static void	join_null(void)
{
	CASE("ft_strjoin(NULL, \"abc\")");
	t_free(ft_strjoin(NULL, "abc"));
	CASE("ft_strjoin(\"abc\", NULL)");
	t_free(ft_strjoin("abc", NULL));
	CASE("ft_strjoin(NULL, NULL)");
	t_free(ft_strjoin(NULL, NULL));
}

static const t_test	g_ft_strjoin[] = {
	TEST("basic join", "Returns a new malloc'd string s1 + s2, with room for the '\\0'.", join_basic),
	TEST("empty s1", "\"\" + \"World\" is a new malloc'd \"World\" (not s2 itself).", join_empty1),
	TEST("empty s2", "\"Hello\" + \"\" is a new malloc'd \"Hello\" (not s1 itself).", join_empty2),
	TEST("both empty", "\"\" + \"\" is a malloc'd \"\" of at least 1 byte.", join_empty),
	TEST("bytes > 127", "Every byte is copied as is.", join_high),
	TEST("s1 and s2 overlap", "The same string (or a part of it) given twice: the inputs are only read.", join_same),
	TEST_SLOW("4 MB + 4 MB", "Two big strings: an int length or a quadratic loop fails here.", join_big),
	TEST_MF("malloc fails", "When its malloc fails, strjoin returns NULL.", mf_strjoin),
	TEST_UB("NULL s1 / s2", "Not defined by the subject, but evaluators try it: no crash.", join_null),
};

/* ================================================================== */
/* ft_strtrim                                                         */
/* ================================================================== */

static void	trim_one(const char *s, const char *set, const char *exp)
{
	char	*g;
	char	*r;

	g = t_gstr(s);
	CASE("ft_strtrim(\"%s\", \"%s\") [guarded]", t_esc(s), t_esc(set));
	r = ft_strtrim(g, t_gstr(set));
	EXPECT(!r || r != g, "must return a new string, not s1");
	if (r != g)
		t_check_str("", r, exp);
	CASE("ft_strtrim(\"%s\", \"%s\") [front guarded]", t_esc(s), t_esc(set));
	r = ft_strtrim(t_gstr_front(s), t_gstr_front(set));
	t_check_str("", r, exp);
}

static void	trim_basic(void)
{
	trim_one("  Hello World  ", " ", "Hello World");
	trim_one("xxhixx", "x", "hi");
	trim_one("\t\n hello \n\t", " \t\n", "hello");
	trim_one("  left", " ", "left");
	trim_one("right  ", " ", "right");
}

static void	trim_all(void)
{
	trim_one("aaaa", "a", "");
	trim_one("a", "a", "");
	trim_one("   xxx   xxx", " x", "");
	trim_one("  \t \t \n   \n\n\n\t", " \n\t", "");
	trim_one("x", "xyz", "");
}

static void	trim_empty_set(void)
{
	trim_one("hi", "", "hi");
	trim_one("  hi  ", "", "  hi  ");
	trim_one("", "", "");
}

static void	trim_empty_s(void)
{
	trim_one("", "abc", "");
	trim_one("", " ", "");
}

static void	trim_middle(void)
{
	trim_one("a b a", " a", "b");
	trim_one("lorem \n ipsum \t dolor \n sit \t amet", " ", "lorem \n ipsum \t dolor \n sit \t amet");
	trim_one("xx a x b xx", "x", " a x b ");
	trim_one("abc", "xyz", "abc");
}

static void	trim_set(void)
{
	trim_one("abcba", "ab", "c");
	trim_one("abcdba", "acb", "d");
	trim_one("lorem ipsum dolor sit amet", "te", "lorem ipsum dolor sit am");
	trim_one("lorem ipsum dolor sit amet", "tel", "orem ipsum dolor sit am");
	trim_one("aabbccXccbbaa", "cba", "X");
}

static void	trim_high(void)
{
	trim_one("\xc8hello\xc8", "\xc8", "hello");
	trim_one("\xff\xfe" "ok\xfe\xff", "\xfe\xff", "ok");
	trim_one("\x80x\x80", "\x80", "x");
}

static void	trim_big(void)
{
	char	*s;
	char	*r;
	size_t	n;

	n = 4 << 20;
	s = big_gstr(n, ' ');
	s[n / 2] = 'X';
	CASE("ft_strtrim(<4 MB of spaces with one 'X' in the middle>, \" \") [guarded]");
	t_check_str("", ft_strtrim(s, " "), "X");
	s = big_gstr(n, 'k');
	CASE("ft_strtrim(<4 MB without anything to trim>, \" \") [guarded]");
	r = ft_strtrim(s, " ");
	EXPECT(r && t_block_size(r) >= n + 1 && r[n - 1] == 'k' && r[n] == 0, "wrong result");
	t_free(r);
}

static void	mf_strtrim(void)
{
	CASE("ft_strtrim(\"  hello  \", \" \"), a malloc fails");
	MF_STR(ft_strtrim(t_gstr("  hello  "), t_gstr(" ")), "hello");
}

static void	mf_strtrim_all(void)
{
	CASE("ft_strtrim(\"xxxx\", \"x\"), a malloc fails");
	MF_STR(ft_strtrim(t_gstr("xxxx"), t_gstr("x")), "");
}

static void	trim_null(void)
{
	CASE("ft_strtrim(NULL, \" \")");
	t_free(ft_strtrim(NULL, " "));
	CASE("ft_strtrim(\"  abc  \", NULL)");
	t_free(ft_strtrim("  abc  ", NULL));
}

static const t_test	g_ft_strtrim[] = {
	TEST("trims both ends", "Chars of set are removed from the start and the end of s1, in a new \
malloc'd string.", trim_basic),
	TEST("everything trimmed gives \"\"", "When every char is in set the result is a malloc'd \"\". The \
end index going below the start index is the classic crash here.", trim_all),
	TEST("empty set: a copy", "With set \"\" nothing is trimmed: a new copy of s1.", trim_empty_set),
	TEST("empty s1", "\"\" trimmed is a malloc'd \"\".", trim_empty_s),
	TEST("chars of set in the middle stay", "Only the ends are trimmed, never the middle.", trim_middle),
	TEST("set with several chars, any order", "Each char of set is removed at the ends, whatever its \
order in set.", trim_set),
	TEST("bytes > 127 in s1 and set", "0xc8, 0xff... must be trimmed too: comparing signed and unsigned \
chars misses them.", trim_high),
	TEST_SLOW("4 MB", "Big strings: nearly everything trimmed, and nothing trimmed.", trim_big),
	TEST_MF("malloc fails", "When its malloc fails, strtrim returns NULL.", mf_strtrim),
	TEST_MF("malloc fails, everything trimmed", "Also when the result is \"\".", mf_strtrim_all),
	TEST_UB("NULL s1 / set", "Not defined by the subject, but evaluators try it: no crash.", trim_null),
};

/* ================================================================== */
/* ft_split                                                           */
/* ================================================================== */

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

#define SPLIT(S, C, ...) do { \
	static const char	*e_[] = {__VA_ARGS__}; \
	CASE("ft_split(\"%s\", %d) [guarded]", t_esc(S), C); \
	check_split(ft_split(t_gstr(S), C), e_); \
	CASE("ft_split(\"%s\", %d) [front guarded]", t_esc(S), C); \
	check_split(ft_split(t_gstr_front(S), C), e_); \
} while (0)

static void	split_basic(void)
{
	SPLIT("a,b,c", ',', "a", "b", "c", NULL);
	SPLIT("hello world", ' ', "hello", "world", NULL);
	SPLIT("lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed non risus.", ' ',
		"lorem", "ipsum", "dolor", "sit", "amet,", "consectetur", "adipiscing", "elit.",
		"Sed", "non", "risus.", NULL);
	SPLIT("0 0 0 0 0 0 0 0 0", ' ', "0", "0", "0", "0", "0", "0", "0", "0", "0", NULL);
}

static void	split_empty(void)
{
	SPLIT("", ',', NULL);
	SPLIT("", ' ', NULL);
	SPLIT("", '\0', NULL);
}

static void	split_only(void)
{
	SPLIT(",,,", ',', NULL);
	SPLIT("x", 'x', NULL);
	SPLIT("          ", ' ', NULL);
}

static void	split_edges(void)
{
	SPLIT(",a,,b,", ',', "a", "b", NULL);
	SPLIT("  hello   world  ", ' ', "hello", "world", NULL);
	SPLIT("      split       this for   me  !       ", ' ', "split", "this", "for", "me", "!", NULL);
	SPLIT("\t\t\t\thello!\t\t\t\t", '\t', "hello!", NULL);
	SPLIT("^^^1^^2a,^^^^3^^^^--h^^^^", '^', "1", "2a,", "3", "--h", NULL);
	SPLIT("xax", 'x', "a", NULL);
}

static void	split_noc(void)
{
	SPLIT("abc", ',', "abc", NULL);
	SPLIT("nonempty", ' ', "nonempty", NULL);
	SPLIT("a", 'x', "a", NULL);
	SPLIT("hello world", ',', "hello world", NULL);
}

static void	split_nul(void)
{
	SPLIT("abc", '\0', "abc", NULL);
	SPLIT("a b c", '\0', "a b c", NULL);
}

static void	split_high(void)
{
	SPLIT("a\xc8" "b\xc8\xc8" "c", (char)0xc8, "a", "b", "c", NULL);
	SPLIT("\xff\xff" "one\xff" "two\xff", (char)0xff, "one", "two", NULL);
	SPLIT("caf\xc3\xa9 cr\xc3\xa8me", ' ', "caf\xc3\xa9", "cr\xc3\xa8me", NULL);
}

static void	split_many(void)
{
	size_t	words;
	size_t	i;
	char	*s;
	char	**r;

	words = 100000;
	s = t_gmem(NULL, words * 3 + 1);
	i = 0;
	while (i < words)
	{
		s[i * 3] = 'a' + i % 26;
		s[i * 3 + 1] = 'z' - i % 26;
		s[i * 3 + 2] = ' ';
		i++;
	}
	s[words * 3] = 0;
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
}

static void	split_bigword(void)
{
	char	*s;
	char	**r;
	size_t	n;

	n = 4 << 20;
	s = big_gstr(n, 'w');
	CASE("ft_split(<4 MB without delimiter>, ' ') [guarded]");
	r = ft_split(s, ' ');
	EXPECT(r && r[0] && t_block_size(r[0]) >= n + 1 && r[0][n - 1] == 'w' && r[0][n] == 0 && !r[1],
		"wrong result");
	t_free_split(r);
}

static void	split_scenario(const char *s, char c)
{
	char	**r;
	size_t	i;

	CASE("ft_split(\"%s\", '%c'), a malloc fails", t_esc(s), c);
	t_arm();
	r = ft_split(t_gstr(s), c);
	t_disarm();
	if (t_injected())
		EXPECT(r == NULL, "returned non-NULL although one of its mallocs returned NULL"
			" (the subject: NULL if ANY allocation fails, and free what was allocated)");
	else if (!r)
		t_fail("returned NULL");
	else
	{
		i = 0;
		while (t_is_block(r[i]))
			i++;
		EXPECT(r[i] == NULL, "the array is not NULL-terminated");
	}
	t_free_split(r);
}

static void	mf_split_words(void)
{
	split_scenario("  lorem ipsum dolor sit amet consectetur adipiscing  ", ' ');
}

static void	mf_split_edges(void)
{
	split_scenario(",,a,,b,,", ',');
}

static void	mf_split_one(void)
{
	split_scenario("single", ' ');
}

static void	mf_split_none(void)
{
	split_scenario("   ", ' ');
}

static void	split_null(void)
{
	char	**r;

	CASE("ft_split(NULL, ' ')");
	r = ft_split(NULL, ' ');
	EXPECT(r == NULL, "should return NULL");
	t_free_split(r);
}

static const t_test	g_ft_split[] = {
	TEST("basic split", "A malloc'd array of malloc'd words, ended by NULL.", split_basic),
	TEST("empty string: { NULL }", "\"\" gives an array holding only NULL (not NULL itself, not { \"\", \
NULL }).", split_empty),
	TEST("only delimiters: { NULL }", "No word at all: an array holding only NULL.", split_only),
	TEST("leading, trailing, repeated delimiters", "Delimiters at the start, at the end and several in a \
row never make empty words.", split_edges),
	TEST("no delimiter: one word", "Without the delimiter the whole string is one word.", split_noc),
	TEST("c = '\\0'", "With c = '\\0' the whole string is one word (and \"\" gives { NULL }).", split_nul),
	TEST("delimiter > 127", "c = (char)0xc8 must split on the byte 0xc8.", split_high),
	TEST_SLOW("100 000 words", "A lot of words: a quadratic count or a recursive split fails here.", split_many),
	TEST_SLOW("one 4 MB word", "A big word without delimiter.", split_bigword),
	TEST_MF("malloc fails, 7 words", "When ANY malloc fails (the array or any word), split frees \
everything it allocated and returns NULL.", mf_split_words),
	TEST_MF("malloc fails, \",,a,,b,,\"", "Same, with delimiters everywhere.", mf_split_edges),
	TEST_MF("malloc fails, 1 word", "Same with a single word.", mf_split_one),
	TEST_MF("malloc fails, no word", "Same when the array holds only NULL.", mf_split_none),
	TEST_UB("NULL s", "Not defined by the subject, returning NULL is the usual choice.", split_null),
};

/* ================================================================== */
/* ft_itoa                                                            */
/* ================================================================== */

static void	check_itoa(int n)
{
	char	exp[16];

	snprintf(exp, sizeof(exp), "%d", n);
	CASE("ft_itoa(%d)", n);
	t_check_str("", ft_itoa(n), exp);
}

static void	itoa_zero(void)
{
	check_itoa(0);
}

static void	itoa_small(void)
{
	int	v;

	v = -100;
	while (v <= 100)
		check_itoa(v++);
}

static void	itoa_min(void)
{
	check_itoa(INT_MIN);
	check_itoa(INT_MIN + 1);
}

static void	itoa_max(void)
{
	check_itoa(INT_MAX);
	check_itoa(INT_MAX - 1);
}

static void	itoa_pow(void)
{
	long	p;

	p = 1;
	while (p <= 1000000000)
	{
		check_itoa((int)p);
		check_itoa((int)p - 1);
		check_itoa((int)(-p));
		check_itoa((int)(-p + 1));
		p *= 10;
	}
}

static void	itoa_random(void)
{
	unsigned int	x;
	size_t			i;

	x = 42;
	i = 0;
	while (i < 20000)
	{
		x = x * 1103515245u + 12345u;
		check_itoa((int)x >> (i % 31));
		i++;
	}
}

static void	mf_itoa_min(void)
{
	CASE("ft_itoa(INT_MIN), a malloc fails");
	MF_STR(ft_itoa(INT_MIN), "-2147483648");
}

static void	mf_itoa_zero(void)
{
	CASE("ft_itoa(0), a malloc fails");
	MF_STR(ft_itoa(0), "0");
}

static const t_test	g_ft_itoa[] = {
	TEST("0", "0 has one digit: \"0\" (a loop while (n > 0) returns \"\").", itoa_zero),
	TEST("-100..100", "Small positive and negative numbers, with the right size malloc'd.", itoa_small),
	TEST("INT_MIN", "-2147483648 can't be negated in an int: -n overflows.", itoa_min),
	TEST("INT_MAX", "2147483647, the longest positive number.", itoa_max),
	TEST("powers of ten and neighbours", "9, 10, 99, 100, ...: the digit count is where off-by-ones hide.", itoa_pow),
	TEST("20 000 random values", "Random numbers of every length, compared with printf.", itoa_random),
	TEST_MF("malloc fails, INT_MIN", "When its malloc fails, itoa returns NULL.", mf_itoa_min),
	TEST_MF("malloc fails, 0", "Same for 0 (often a separate branch).", mf_itoa_zero),
};

/* ================================================================== */
/* ft_strmapi / ft_striteri                                           */
/* ================================================================== */

static unsigned int	g_idx[64];
static char			g_chars[64];
static char			*g_ptrs[64];
static int			g_ncalls;

static char	map_rec(unsigned int i, char c)
{
	if (g_ncalls < 64)
	{
		g_idx[g_ncalls] = i;
		g_chars[g_ncalls] = c;
	}
	g_ncalls++;
	return (c + 1);
}

static char	map_rot(unsigned int i, char c)
{
	return (c + (char)(i % 3));
}

static void	iter_rec(unsigned int i, char *c)
{
	if (g_ncalls < 64)
	{
		g_idx[g_ncalls] = i;
		g_ptrs[g_ncalls] = c;
	}
	g_ncalls++;
	*c = (char)toupper((unsigned char)*c);
}

static void	iter_next(unsigned int i, char *c)
{
	(void)i;
	g_ncalls++;
	*c = *c + 1;
}

static void	mapi_basic(void)
{
	CASE("ft_strmapi(\"abc\", c + 1) [guarded]");
	t_check_str("", ft_strmapi(t_gstr("abc"), map_rec), "bcd");
	CASE("ft_strmapi(\"hello\", c + 1) [front guarded]");
	t_check_str("", ft_strmapi(t_gstr_front("hello"), map_rec), "ifmmp");
	CASE("ft_strmapi(\"aaaaaa\", c + i %% 3)");
	t_check_str("", ft_strmapi(t_gstr("aaaaaa"), map_rot), "abcabc");
}

static void	mapi_calls(void)
{
	int	i;
	int	ok;

	g_ncalls = 0;
	CASE("ft_strmapi(\"hello\", f): f's arguments");
	t_free(ft_strmapi(t_gstr("hello"), map_rec));
	ok = g_ncalls == 5;
	i = 0;
	while (ok && i < 5)
	{
		ok = g_idx[i] == (unsigned)i && g_chars[i] == "hello"[i];
		i++;
	}
	EXPECT(ok, "f must be called once per char with (0, 'h'), (1, 'e') ... in order (called %d time(s))",
		g_ncalls);
}

static void	mapi_empty(void)
{
	g_ncalls = 0;
	CASE("ft_strmapi(\"\", f) [guarded]");
	t_check_str("", ft_strmapi(t_gstr(""), map_rec), "");
	EXPECT(g_ncalls == 0, "f must not be called for \"\"");
}

static void	mapi_orig(void)
{
	char	*s;
	char	*r;

	s = t_gstr("hello");
	CASE("ft_strmapi(s = \"hello\", f): s itself");
	r = ft_strmapi(s, map_rec);
	EXPECT(!strcmp(s, "hello"), "the original string was modified: \"%s\"", t_esc(s));
	EXPECT(r != s, "must return a new string");
	if (r != s)
		t_free(r);
}

static void	mapi_high(void)
{
	g_ncalls = 0;
	CASE("ft_strmapi(\"\\xc8\\xff\", c + 1)");
	t_check_str("", ft_strmapi(t_gstr("\xc8\xff"), map_rec), "\xc9");
	EXPECT(g_chars[0] == '\xc8', "f must get the char 0xc8");
}

static void	mapi_big(void)
{
	char	*s;
	char	*r;
	size_t	n;

	n = 4 << 20;
	s = big_gstr(n, 'a');
	g_ncalls = 0;
	CASE("ft_strmapi(<4 MB>, f) [guarded]");
	r = ft_strmapi(s, map_rec);
	EXPECT(r && t_block_size(r) >= n + 1 && r[0] == 'b' && r[n - 1] == 'b' && r[n] == 0
		&& g_ncalls == (int)n, "wrong result");
	t_free(r);
}

static void	mf_strmapi(void)
{
	CASE("ft_strmapi(\"abcdef\", f), a malloc fails");
	MF_STR(ft_strmapi(t_gstr("abcdef"), map_rot), "acedfh");
}

static void	mapi_null(void)
{
	CASE("ft_strmapi(NULL, f)");
	t_free(ft_strmapi(NULL, map_rec));
	CASE("ft_strmapi(\"abc\", NULL)");
	t_free(ft_strmapi("abc", NULL));
}

static const t_test	g_ft_strmapi[] = {
	TEST("applies f to every char", "The result is a new malloc'd string of f(i, s[i]).", mapi_basic),
	TEST("f gets (index, char), in order", "f is called once per char, with the index from 0 and the \
char, from left to right.", mapi_calls),
	TEST("empty string", "\"\" gives a malloc'd \"\" and f is never called.", mapi_empty),
	TEST("s is not modified", "strmapi writes into a new string, never into s.", mapi_orig),
	TEST("bytes > 127", "The chars are passed as they are (0xc8 stays 0xc8), and the result is \
'\\0'-terminated even when f makes a 0.", mapi_high),
	TEST_SLOW("4 MB", "A big string: f called 4 million times, one malloc.", mapi_big),
	TEST_MF("malloc fails", "When its malloc fails, strmapi returns NULL.", mf_strmapi),
	TEST_UB("NULL s / f", "Not defined by the subject, but evaluators try it: no crash.", mapi_null),
};

static void	iteri_basic(void)
{
	char	*s;

	s = t_gstr("hello");
	CASE("ft_striteri(\"hello\", toupper) [guarded]");
	ft_striteri(s, iter_rec);
	EXPECT(!strcmp(s, "HELLO"), "expected \"HELLO\", got \"%s\"", t_esc(s));
	s = t_gstr_front("abc");
	CASE("ft_striteri(\"abc\", toupper) [front guarded]");
	ft_striteri(s, iter_rec);
	EXPECT(!strcmp(s, "ABC"), "expected \"ABC\", got \"%s\"", t_esc(s));
}

static void	iteri_ptr(void)
{
	char	*s;
	int		i;
	int		ok;

	s = t_gstr("hello");
	g_ncalls = 0;
	CASE("ft_striteri(\"hello\", f): f's arguments");
	ft_striteri(s, iter_rec);
	ok = g_ncalls == 5;
	i = 0;
	while (ok && i < 5)
	{
		ok = g_ptrs[i] == s + i;
		i++;
	}
	EXPECT(ok, "f must get &s[i] (the address inside s, not a copy), called %d time(s)", g_ncalls);
}

static void	iteri_index(void)
{
	char	*s;
	int		i;
	int		ok;

	s = t_gstr("abcdefghij");
	g_ncalls = 0;
	CASE("ft_striteri(\"abcdefghij\", f): indexes");
	ft_striteri(s, iter_rec);
	ok = g_ncalls == 10;
	i = 0;
	while (ok && i < 10)
	{
		ok = g_idx[i] == (unsigned)i;
		i++;
	}
	EXPECT(ok, "f must get the indexes 0, 1, 2, ... in order (called %d time(s))", g_ncalls);
}

static void	iteri_empty(void)
{
	g_ncalls = 0;
	CASE("ft_striteri(\"\", f) [guarded]");
	ft_striteri(t_gstr(""), iter_rec);
	EXPECT(g_ncalls == 0, "f must not be called for \"\"");
	CASE("ft_striteri(\"\", f) [front guarded]");
	ft_striteri(t_gstr_front(""), iter_rec);
	EXPECT(g_ncalls == 0, "f must not be called for \"\"");
}

static void	iteri_stop(void)
{
	char	*s;

	s = t_gstr("aaa");
	g_ncalls = 0;
	CASE("ft_striteri(\"aaa\", c + 1) [guarded]");
	ft_striteri(s, iter_next);
	EXPECT(!strcmp(s, "bbb") && g_ncalls == 3, "expected \"bbb\" after 3 calls, got \"%s\" after %d",
		t_esc(s), g_ncalls);
}

static void	iteri_big(void)
{
	char	*s;
	size_t	n;

	n = 4 << 20;
	s = big_gstr(n, 'a');
	g_ncalls = 0;
	CASE("ft_striteri(<4 MB>, c + 1) [guarded]");
	ft_striteri(s, iter_next);
	EXPECT(g_ncalls == (int)n && s[0] == 'b' && s[n - 1] == 'b', "wrong result");
}

static void	iteri_null(void)
{
	CASE("ft_striteri(NULL, f)");
	ft_striteri(NULL, iter_rec);
	CASE("ft_striteri(\"abc\", NULL)");
	ft_striteri(t_gstr("abc"), NULL);
}

static const t_test	g_ft_striteri[] = {
	TEST("modifies s in place", "f changes each char through its pointer: s itself is modified.", iteri_basic),
	TEST("f gets &s[i]", "The second argument is the address of the char inside s, not a copy.", iteri_ptr),
	TEST("f gets the indexes 0, 1, 2...", "The first argument is the index, from 0, in order.", iteri_index),
	TEST("empty string: f never called", "\"\" has no char to iterate on.", iteri_empty),
	TEST("stops at the '\\0' [guarded]", "Exactly strlen(s) calls, nothing after the '\\0' (protected page).", iteri_stop),
	TEST_SLOW("4 MB", "A big string, f called 4 million times.", iteri_big),
	TEST_UB("NULL s / f", "Not defined by the subject, but evaluators try it: no crash.", iteri_null),
};

/* ================================================================== */
/* ft_put*_fd                                                         */
/* ================================================================== */

static int	g_saved_out = -1;

/* stdout goes to a capture file until stdout_release */
static int	stdout_catch(void)
{
	int	fd;

	fd = t_capture_open();
	fflush(stdout);
	g_saved_out = dup(STDOUT_FILENO);
	dup2(fd, STDOUT_FILENO);
	return (fd);
}

static size_t	stdout_release(int fd, char *buf, size_t size)
{
	size_t	n;

	fflush(stdout);
	dup2(g_saved_out, STDOUT_FILENO);
	close(g_saved_out);
	n = t_capture_read(fd, buf, size);
	close(fd);
	return (n);
}

static void	putchar_one(int fd, char c)
{
	char	buf[16];
	size_t	n;

	CASE("ft_putchar_fd(%d, fd)", c);
	ft_putchar_fd(c, fd);
	n = t_capture_read(fd, buf, sizeof(buf));
	EXPECT(n == 1 && buf[0] == c, "expected exactly 1 byte (0x%02x), got %zu byte(s)%s", (unsigned char)c, n,
		n == 1 ? " with another value" : "");
}

static void	putchar_basic(void)
{
	int	fd;

	fd = t_capture_open();
	putchar_one(fd, 'a');
	putchar_one(fd, 'Z');
	putchar_one(fd, '4');
	close(fd);
}

static void	putchar_nul(void)
{
	int	fd;

	fd = t_capture_open();
	putchar_one(fd, '\0');
	close(fd);
}

static void	putchar_special(void)
{
	int	fd;

	fd = t_capture_open();
	putchar_one(fd, '\n');
	putchar_one(fd, '\t');
	putchar_one(fd, 127);
	close(fd);
}

static void	putchar_high(void)
{
	int	fd;

	fd = t_capture_open();
	putchar_one(fd, (char)200);
	putchar_one(fd, (char)255);
	putchar_one(fd, (char)128);
	close(fd);
}

static void	putchar_many(void)
{
	int		fd;
	char	buf[1200];
	size_t	n;
	int		i;
	int		ok;

	fd = t_capture_open();
	CASE("1000 x ft_putchar_fd(i %% 256, fd)");
	i = 0;
	while (i < 1000)
	{
		ft_putchar_fd((char)i, fd);
		i++;
	}
	n = t_capture_read(fd, buf, sizeof(buf));
	ok = n == 1000;
	i = 0;
	while (ok && i < 1000)
	{
		ok = buf[i] == (char)i;
		i++;
	}
	EXPECT(ok, "expected the 1000 bytes in order, got %zu byte(s)", n);
	close(fd);
}

static void	putchar_notstdout(void)
{
	int		cap;
	int		fd;
	char	buf[16];
	size_t	n;

	fd = t_capture_open();
	cap = stdout_catch();
	CASE("ft_putchar_fd('x', fd) with fd != 1");
	ft_putchar_fd('x', fd);
	n = stdout_release(cap, buf, sizeof(buf));
	EXPECT(n == 0, "wrote %zu byte(s) to stdout instead of fd", n);
	n = t_capture_read(fd, buf, sizeof(buf));
	EXPECT(n == 1 && buf[0] == 'x', "expected 'x' on fd");
	close(fd);
}

static void	putchar_bad(void)
{
	CASE("ft_putchar_fd('a', -1)");
	ft_putchar_fd('a', -1);
	CASE("ft_putchar_fd('a', 4242) (fd not open)");
	ft_putchar_fd('a', 4242);
}

static const t_test	g_ft_putchar_fd[] = {
	TEST("writes one char", "Exactly one byte, the char, on fd.", putchar_basic),
	TEST("writes '\\0'", "'\\0' is a char like the others: one byte 0x00 is written.", putchar_nul),
	TEST("\\n, \\t, DEL", "Control chars are written as they are.", putchar_special),
	TEST("bytes > 127", "(char)200 is written as the single byte 0xc8, not as a UTF-8 sequence.", putchar_high),
	TEST("1000 chars in a row", "Every call writes exactly its char, in order.", putchar_many),
	TEST("writes on fd, not on stdout", "The fd argument must be used: write(1, ...) is a KO.", putchar_notstdout),
	TEST("invalid fd does not crash", "write() fails on fd -1 or a closed fd: nothing to do, no crash.", putchar_bad),
};

static size_t	put_str(void (*f)(char *, int), const char *s, int front, char *buf, size_t size)
{
	int		fd;
	size_t	n;

	fd = t_capture_open();
	f(front ? t_gstr_front(s) : t_gstr(s), fd);
	n = t_capture_read(fd, buf, size);
	close(fd);
	return (n);
}

static void	putstr_basic(void)
{
	char	buf[64];
	size_t	n;

	CASE("ft_putstr_fd(\"hello\", fd) [guarded]");
	n = put_str(ft_putstr_fd, "hello", 0, buf, sizeof(buf));
	EXPECT(n == 5 && !memcmp(buf, "hello", 5), "got \"%s\"", t_escn(buf, n));
	CASE("ft_putstr_fd(\"Hello World 42!\", fd) [front guarded]");
	n = put_str(ft_putstr_fd, "Hello World 42!", 1, buf, sizeof(buf));
	EXPECT(n == 15 && !memcmp(buf, "Hello World 42!", 15), "got \"%s\"", t_escn(buf, n));
}

static void	putstr_empty(void)
{
	char	buf[16];
	size_t	n;

	CASE("ft_putstr_fd(\"\", fd) [guarded]");
	n = put_str(ft_putstr_fd, "", 0, buf, sizeof(buf));
	EXPECT(n == 0, "wrote %zu byte(s) for \"\"", n);
}

static void	putstr_special(void)
{
	char	buf[64];
	size_t	n;

	CASE("ft_putstr_fd(\"\\xff\\x80 ok\\n\\t\", fd)");
	n = put_str(ft_putstr_fd, "\xff\x80 ok\n\t", 0, buf, sizeof(buf));
	EXPECT(n == 7 && !memcmp(buf, "\xff\x80 ok\n\t", 7), "got \"%s\"", t_escn(buf, n));
}

static void	putstr_guard(void)
{
	char	src[80];
	char	buf[80];
	size_t	len;
	size_t	n;

	len = 0;
	while (len < 70)
	{
		memset(src, 'a' + len % 26, len);
		src[len] = 0;
		CASE("ft_putstr_fd(<%zu chars>, fd) [guarded]", len);
		n = put_str(ft_putstr_fd, src, 0, buf, sizeof(buf));
		EXPECT(n == len && !memcmp(buf, src, len), "wrote %zu byte(s), expected %zu", n, len);
		len++;
	}
}

static void	putstr_big(void)
{
	int		fd;
	char	*buf;
	size_t	n;
	size_t	sz;

	sz = 200000;
	buf = t_gmem(NULL, 300000);
	fd = t_capture_open();
	CASE("ft_putstr_fd(<200 KB>, fd) [guarded]");
	ft_putstr_fd(big_gstr(sz, 'q'), fd);
	n = t_capture_read(fd, buf, 300000);
	EXPECT(n == sz && buf[0] == 'q' && buf[sz - 1] == 'q', "wrote %zu byte(s), expected %zu", n, sz);
	close(fd);
}

static void	putstr_notstdout(void)
{
	int		cap;
	int		fd;
	char	buf[16];
	size_t	n;

	fd = t_capture_open();
	cap = stdout_catch();
	CASE("ft_putstr_fd(\"abc\", fd) with fd != 1");
	ft_putstr_fd("abc", fd);
	n = stdout_release(cap, buf, sizeof(buf));
	EXPECT(n == 0, "wrote %zu byte(s) to stdout instead of fd", n);
	close(fd);
}

static void	putstr_bad(void)
{
	CASE("ft_putstr_fd(\"abc\", -1)");
	ft_putstr_fd("abc", -1);
	CASE("ft_putstr_fd(\"abc\", 4242) (fd not open)");
	ft_putstr_fd("abc", 4242);
}

static void	putstr_null(void)
{
	CASE("ft_putstr_fd(NULL, fd)");
	ft_putstr_fd(NULL, -1);
}

static const t_test	g_ft_putstr_fd[] = {
	TEST("writes the string", "Every char of s, nothing more (no '\\0', no newline).", putstr_basic),
	TEST("empty string writes nothing", "\"\" writes 0 bytes.", putstr_empty),
	TEST("bytes > 127 and control chars", "Every byte is written as it is.", putstr_special),
	TEST("never past the '\\0' [guarded]", "Lengths 0..69, the string ending at a protected page.", putstr_guard),
	TEST("200 KB", "A big string, written completely.", putstr_big),
	TEST("writes on fd, not on stdout", "The fd argument must be used.", putstr_notstdout),
	TEST("invalid fd does not crash", "write() fails on fd -1 or a closed fd: no crash.", putstr_bad),
	TEST_UB("NULL s", "Not defined by the subject, but evaluators try it: no crash.", putstr_null),
};

static void	putendl_basic(void)
{
	char	buf[64];
	size_t	n;

	CASE("ft_putendl_fd(\"hello\", fd) [guarded]");
	n = put_str(ft_putendl_fd, "hello", 0, buf, sizeof(buf));
	EXPECT(n == 6 && !memcmp(buf, "hello\n", 6), "got \"%s\"", t_escn(buf, n));
	CASE("ft_putendl_fd(\"42\", fd) [front guarded]");
	n = put_str(ft_putendl_fd, "42", 1, buf, sizeof(buf));
	EXPECT(n == 3 && !memcmp(buf, "42\n", 3), "got \"%s\"", t_escn(buf, n));
}

static void	putendl_empty(void)
{
	char	buf[16];
	size_t	n;

	CASE("ft_putendl_fd(\"\", fd) [guarded]");
	n = put_str(ft_putendl_fd, "", 0, buf, sizeof(buf));
	EXPECT(n == 1 && buf[0] == '\n', "expected \"\\n\", got \"%s\"", t_escn(buf, n));
}

static void	putendl_nl(void)
{
	char	buf[16];
	size_t	n;

	CASE("ft_putendl_fd(\"a\\nb\\n\", fd)");
	n = put_str(ft_putendl_fd, "a\nb\n", 0, buf, sizeof(buf));
	EXPECT(n == 5 && !memcmp(buf, "a\nb\n\n", 5), "expected \"a\\nb\\n\\n\", got \"%s\"", t_escn(buf, n));
}

static void	putendl_guard(void)
{
	char	src[80];
	char	buf[80];
	size_t	len;
	size_t	n;

	len = 0;
	while (len < 70)
	{
		memset(src, 'a' + len % 26, len);
		src[len] = 0;
		CASE("ft_putendl_fd(<%zu chars>, fd) [guarded]", len);
		n = put_str(ft_putendl_fd, src, 0, buf, sizeof(buf));
		EXPECT(n == len + 1 && !memcmp(buf, src, len) && buf[len] == '\n', "wrote %zu byte(s), expected %zu",
			n, len + 1);
		len++;
	}
}

static void	putendl_big(void)
{
	int		fd;
	char	*buf;
	size_t	n;
	size_t	sz;

	sz = 200000;
	buf = t_gmem(NULL, 300000);
	fd = t_capture_open();
	CASE("ft_putendl_fd(<200 KB>, fd) [guarded]");
	ft_putendl_fd(big_gstr(sz, 'q'), fd);
	n = t_capture_read(fd, buf, 300000);
	EXPECT(n == sz + 1 && buf[sz - 1] == 'q' && buf[sz] == '\n', "wrote %zu byte(s), expected %zu", n, sz + 1);
	close(fd);
}

static void	putendl_notstdout(void)
{
	int		cap;
	int		fd;
	char	buf[16];
	size_t	n;

	fd = t_capture_open();
	cap = stdout_catch();
	CASE("ft_putendl_fd(\"abc\", fd) with fd != 1");
	ft_putendl_fd("abc", fd);
	n = stdout_release(cap, buf, sizeof(buf));
	EXPECT(n == 0, "wrote %zu byte(s) to stdout instead of fd (the '\\n' too?)", n);
	close(fd);
}

static void	putendl_bad(void)
{
	CASE("ft_putendl_fd(\"abc\", -1)");
	ft_putendl_fd("abc", -1);
	CASE("ft_putendl_fd(\"abc\", 4242) (fd not open)");
	ft_putendl_fd("abc", 4242);
}

static void	putendl_null(void)
{
	CASE("ft_putendl_fd(NULL, fd)");
	ft_putendl_fd(NULL, -1);
}

static const t_test	g_ft_putendl_fd[] = {
	TEST("writes the string + '\\n'", "s followed by exactly one newline.", putendl_basic),
	TEST("empty string writes \"\\n\"", "\"\" still gets its newline.", putendl_empty),
	TEST("newlines inside s", "The newlines of s are written as they are, then one more.", putendl_nl),
	TEST("never past the '\\0' [guarded]", "Lengths 0..69, the string ending at a protected page.", putendl_guard),
	TEST("200 KB", "A big string + '\\n'.", putendl_big),
	TEST("writes on fd, not on stdout", "The string AND the newline go to fd.", putendl_notstdout),
	TEST("invalid fd does not crash", "write() fails on fd -1 or a closed fd: no crash.", putendl_bad),
	TEST_UB("NULL s", "Not defined by the subject, but evaluators try it: no crash.", putendl_null),
};

static void	putnbr_one(int fd, int v)
{
	char	buf[64];
	char	exp[16];
	size_t	n;

	snprintf(exp, sizeof(exp), "%d", v);
	CASE("ft_putnbr_fd(%d, fd)", v);
	ft_putnbr_fd(v, fd);
	n = t_capture_read(fd, buf, sizeof(buf));
	EXPECT(n == strlen(exp) && !memcmp(buf, exp, n), "expected \"%s\", got \"%s\"", exp, t_escn(buf, n));
}

static void	putnbr_zero(void)
{
	int	fd;

	fd = t_capture_open();
	putnbr_one(fd, 0);
	close(fd);
}

static void	putnbr_small(void)
{
	int	fd;
	int	v;

	fd = t_capture_open();
	v = -100;
	while (v <= 100)
		putnbr_one(fd, v++);
	close(fd);
}

static void	putnbr_min(void)
{
	int	fd;

	fd = t_capture_open();
	putnbr_one(fd, INT_MIN);
	putnbr_one(fd, INT_MIN + 1);
	close(fd);
}

static void	putnbr_max(void)
{
	int	fd;

	fd = t_capture_open();
	putnbr_one(fd, INT_MAX);
	putnbr_one(fd, INT_MAX - 1);
	close(fd);
}

static void	putnbr_pow(void)
{
	int		fd;
	long	p;

	fd = t_capture_open();
	p = 1;
	while (p <= 1000000000)
	{
		putnbr_one(fd, (int)p);
		putnbr_one(fd, (int)p - 1);
		putnbr_one(fd, (int)-p);
		p *= 10;
	}
	close(fd);
}

static void	putnbr_random(void)
{
	int				fd;
	unsigned int	x;
	size_t			i;

	fd = t_capture_open();
	x = 7;
	i = 0;
	while (i < 5000)
	{
		x = x * 1103515245u + 12345u;
		putnbr_one(fd, (int)x >> (i % 31));
		i++;
	}
	close(fd);
}

static void	putnbr_notstdout(void)
{
	int		cap;
	int		fd;
	char	buf[16];
	size_t	n;

	fd = t_capture_open();
	cap = stdout_catch();
	CASE("ft_putnbr_fd(-42, fd) with fd != 1");
	ft_putnbr_fd(-42, fd);
	n = stdout_release(cap, buf, sizeof(buf));
	EXPECT(n == 0, "wrote %zu byte(s) to stdout instead of fd", n);
	close(fd);
}

static void	putnbr_bad(void)
{
	CASE("ft_putnbr_fd(INT_MIN, -1)");
	ft_putnbr_fd(INT_MIN, -1);
	CASE("ft_putnbr_fd(42, 4242) (fd not open)");
	ft_putnbr_fd(42, 4242);
}

static const t_test	g_ft_putnbr_fd[] = {
	TEST("0", "0 is written as \"0\" (a loop while (n > 0) writes nothing).", putnbr_zero),
	TEST("-100..100", "Small numbers, with their '-'.", putnbr_small),
	TEST("INT_MIN", "\"-2147483648\": -n overflows in an int.", putnbr_min),
	TEST("INT_MAX", "\"2147483647\".", putnbr_max),
	TEST("powers of ten and neighbours", "9, 10, 99, 100, ...: no digit lost or added.", putnbr_pow),
	TEST("5000 random values", "Compared with printf; any malloc must be freed (leaks are checked).", putnbr_random),
	TEST("writes on fd, not on stdout", "The fd argument must be used, also for the '-'.", putnbr_notstdout),
	TEST("invalid fd does not crash", "write() fails on fd -1 or a closed fd: no crash.", putnbr_bad),
};

/* ================================================================== */

void	run_part2(void)
{
	t_section("PART 2 - ADDITIONAL FUNCTIONS");
	GROUP("ft_substr", g_ft_substr);
	GROUP("ft_strjoin", g_ft_strjoin);
	GROUP("ft_strtrim", g_ft_strtrim);
	GROUP("ft_split", g_ft_split);
	GROUP("ft_itoa", g_ft_itoa);
	GROUP("ft_strmapi", g_ft_strmapi);
	GROUP("ft_striteri", g_ft_striteri);
	GROUP("ft_putchar_fd", g_ft_putchar_fd);
	GROUP("ft_putstr_fd", g_ft_putstr_fd);
	GROUP("ft_putendl_fd", g_ft_putendl_fd);
	GROUP("ft_putnbr_fd", g_ft_putnbr_fd);
}
