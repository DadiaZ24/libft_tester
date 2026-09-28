#include "tester.h"

/*
** PART 1 - Libc functions
** Every result is compared with the real libc (or with a reference
** implementation for the BSD-only strlcpy / strlcat / strnstr).
** "[guarded]" means the input ends (or starts, "front guarded") exactly at a
** PROT_NONE page: reading or writing a single byte too far is a SEGFAULT.
*/

static int	sign(int x)
{
	return ((x > 0) - (x < 0));
}

static const char	*sgn(int x)
{
	return (x < 0 ? "< 0" : x > 0 ? "> 0" : "== 0");
}

static size_t	ref_strlcpy(char *dst, const char *src, size_t size)
{
	size_t	len;

	len = strlen(src);
	if (size)
	{
		size = (len < size - 1) ? len : size - 1;
		memcpy(dst, src, size);
		dst[size] = 0;
	}
	return (len);
}

static size_t	ref_strlcat(char *dst, const char *src, size_t size)
{
	size_t	dlen;

	dlen = 0;
	while (dlen < size && dst[dlen])
		dlen++;
	if (dlen == size)
		return (size + strlen(src));
	return (dlen + ref_strlcpy(dst + dlen, src, size - dlen));
}

static const char	*ref_strnstr(const char *big, const char *little, size_t len)
{
	size_t	l;
	size_t	i;

	l = strlen(little);
	if (!l)
		return (big);
	i = 0;
	while (big[i] && i < len && len - i >= l)
	{
		if (strncmp(big + i, little, l) == 0)
			return (big + i);
		i++;
	}
	return (NULL);
}

static void	fill_pattern(unsigned char *p, size_t n, unsigned seed)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		p[i] = (unsigned char)(i * 7 + seed);
		i++;
	}
}

static int	all_bytes(const unsigned char *p, size_t n, unsigned char c)
{
	size_t	i;

	i = 0;
	while (i < n)
		if (p[i++] != c)
			return (0);
	return (1);
}

/* a string of n chars ending exactly at a protected page */
static char	*big_gstr(size_t n, char c)
{
	char	*s;

	s = t_gmem(NULL, n + 1);
	memset(s, c, n);
	s[n] = 0;
	return (s);
}

/* ================================================================== */
/* ft_is* / ft_to*                                                    */
/* ================================================================== */

static int	w_isalpha(int c) { return (isalpha(c)); }
static int	w_isdigit(int c) { return (isdigit(c)); }
static int	w_isalnum(int c) { return (isalnum(c)); }
static int	w_isascii(int c) { return (c >= 0 && c <= 127); }
static int	w_isprint(int c) { return (isprint(c)); }
static int	w_toupper(int c) { return (toupper(c)); }
static int	w_tolower(int c) { return (tolower(c)); }

static const int	g_edges[] = {'/', '0', '9', ':', '@', 'A', 'Z', '[', '`', 'a', 'z', '{',
	0, 1, 8, 9, 10, 13, 31, 32, 33, 126, 127, 128};

static void	ct_range(const char *name, int (*ft)(int), int (*ref)(int), int lo, int hi, int exact)
{
	int	c;
	int	got;
	int	exp;

	c = lo;
	while (c <= hi)
	{
		CASE("%s(%d)", name, c);
		got = ft(c);
		exp = ref(c) ? 1 : 0;
		if (exact)
			EXPECT(got == exp, "expected exactly %d, got %d", exp, got);
		else
			EXPECT(!!got == exp, "expected %s, got %d", exp ? "true (non-zero)" : "false (0)", got);
		c++;
	}
}

static void	ct_edges(const char *name, int (*ft)(int), int (*ref)(int))
{
	size_t	i;
	int		got;
	int		exp;

	i = 0;
	while (i < sizeof(g_edges) / sizeof(*g_edges))
	{
		CASE("%s(%d '%s')", name, g_edges[i], t_escn((char []){(char)g_edges[i]}, 1));
		got = ft(g_edges[i]);
		exp = ref(g_edges[i]) ? 1 : 0;
		EXPECT(!!got == exp, "expected %s, got %d", exp ? "true" : "false", got);
		i++;
	}
}

static const int	g_outside[] = {-2, -56, -128, -129, -200, -255, -256, -300, -1000, 256, 257,
	'a' + 256, '0' + 256, 300, 511, 512, 1000, 65536, INT_MAX, INT_MIN, INT_MIN + 1, INT_MAX - 1};

/* UB: must not crash; ft_is* should say 0, ft_to* should return c */
static void	ct_outside(const char *name, int (*ft)(int), int is_conv)
{
	size_t	i;
	int		c;
	int		got;

	i = 0;
	while (i < sizeof(g_outside) / sizeof(*g_outside))
	{
		c = g_outside[i++];
		CASE("%s(%d)", name, c);
		got = ft(c);
		if (is_conv)
			EXPECT(got == c, "should return %d unchanged, got %d", c, got);
		else
			EXPECT(got == 0, "should be 0, got %d", got);
	}
}

static void	ct_members(const char *name, int (*ft)(int), int (*ref)(int))
{
	int	c;
	int	got;

	c = 0;
	while (c <= 255)
	{
		if (ref(c))
		{
			CASE("%s(%d '%s')", name, c, t_escn((char []){(char)c}, 1));
			got = ft(c);
			EXPECT(got == 1, "a member of the class: expected 1, got %d", got);
		}
		c++;
	}
}

static void	cv_range(const char *name, int (*ft)(int), int (*ref)(int), int lo, int hi)
{
	int	c;

	c = lo;
	while (c <= hi)
	{
		CASE("%s(%d)", name, c);
		EXPECT(ft(c) == ref(c), "expected %d, got %d", ref(c), ft(c));
		c++;
	}
}

static void	cv_edges(const char *name, int (*ft)(int), int (*ref)(int))
{
	size_t	i;
	int		c;

	i = 0;
	while (i < sizeof(g_edges) / sizeof(*g_edges))
	{
		c = g_edges[i++];
		CASE("%s(%d)", name, c);
		EXPECT(ft(c) == ref(c), "expected %d, got %d", ref(c), ft(c));
	}
}

#define WHY_CT1 "Every ASCII character (0..127) must be classified like the libc function does in the \
C locale. A wrong range (e.g. using 'z' + 1, or forgetting the upper case) fails here."
#define WHY_CT2 "The subject asks for exactly 1 (true) or 0 (false), for every value from -1 to 255. \
Returning 1024 (like the glibc) or c itself is a KO here."
#define WHY_CT3 "Characters right at the edges of each range: '/' '0' '9' ':' '@' 'A' 'Z' '[' '`' 'a' \
'z' '{', 31 32 126 127 128. An off-by-one (< instead of <=) fails here."
#define WHY_CT4 "Bytes 128..255 (accented letters in Latin-1, UTF-8 bytes...) are not ASCII: in the C \
locale they are never letters, digits or printable, and never ASCII."
#define WHY_CT5 "EOF (-1) is a valid argument of every ctype function and must give 0. An \
implementation that indexes a table with c reads before the table."
#define WHY_CT6 "Values outside -1..255 are undefined behaviour, but evaluators try them (a \
negative char like -56, INT_MIN, 256 + 'a'): no crash, and 0 is expected."

#define CTYPE(FN, REF) \
static void	FN##_1(void) { ct_range(#FN, FN, REF, 0, 127, 0); } \
static void	FN##_2(void) { ct_range(#FN, FN, REF, -1, 255, 1); } \
static void	FN##_3(void) { ct_edges(#FN, FN, REF); } \
static void	FN##_4(void) { ct_range(#FN, FN, REF, 128, 255, 0); } \
static void	FN##_5(void) { ct_range(#FN, FN, REF, -1, -1, 1); } \
static void	FN##_6(void) { ct_members(#FN, FN, REF); } \
static void	FN##_7(void) { ct_outside(#FN, FN, 0); } \
static const t_test	g_##FN[] = { \
	TEST("ASCII 0..127 classified like libc", WHY_CT1, FN##_1), \
	TEST("returns exactly 1 or 0 (-1..255)", WHY_CT2, FN##_2), \
	TEST("edges of every range", WHY_CT3, FN##_3), \
	TEST("bytes 128..255 are false", WHY_CT4, FN##_4), \
	TEST("EOF (-1) is false", WHY_CT5, FN##_5), \
	TEST("every member of the class gives 1", "Every character of the class (as the libc sees it) must \
return exactly 1, from the first one to the last one.", FN##_6), \
	TEST_UB("outside -1..255: no crash, 0", WHY_CT6, FN##_7), \
};

CTYPE(ft_isalpha, w_isalpha)
CTYPE(ft_isdigit, w_isdigit)
CTYPE(ft_isalnum, w_isalnum)
CTYPE(ft_isascii, w_isascii)
CTYPE(ft_isprint, w_isprint)

#define WHY_CV1 "Every letter of the other case must be converted: a..z -> A..Z for toupper, A..Z -> \
a..z for tolower."
#define WHY_CV2 "Everything else in ASCII (digits, punctuation, control chars, letters already in the \
right case) must be returned unchanged."
#define WHY_CV3 "Characters right at the edges of the letter ranges: '@' and '[' surround A..Z, '`' \
and '{' surround a..z. An off-by-one converts them."
#define WHY_CV4 "In the C locale only the ASCII letters are converted: 128..255 are returned as they are."
#define WHY_CV5 "EOF (-1) is a valid argument and must be returned unchanged."
#define WHY_CV6 "Values outside -1..255 are undefined behaviour, but evaluators try them: no crash, and \
c returned unchanged is expected."

#define CONV(FN, REF, LO, HI) \
static void	FN##_1(void) { cv_range(#FN, FN, REF, LO, HI); } \
static void	FN##_2(void) { cv_range(#FN, FN, REF, 0, 127); } \
static void	FN##_3(void) { cv_edges(#FN, FN, REF); } \
static void	FN##_4(void) { cv_range(#FN, FN, REF, 128, 255); } \
static void	FN##_5(void) { cv_range(#FN, FN, REF, -1, -1); } \
static void	FN##_6(void) { ct_outside(#FN, FN, 1); } \
static const t_test	g_##FN[] = { \
	TEST("letters converted (" #LO ".." #HI ")", WHY_CV1, FN##_1), \
	TEST("the rest of ASCII unchanged", WHY_CV2, FN##_2), \
	TEST("edges of the letter ranges", WHY_CV3, FN##_3), \
	TEST("bytes 128..255 unchanged", WHY_CV4, FN##_4), \
	TEST("EOF (-1) unchanged", WHY_CV5, FN##_5), \
	TEST_UB("outside -1..255: no crash, unchanged", WHY_CV6, FN##_6), \
};

CONV(ft_toupper, w_toupper, 'a', 'z')
CONV(ft_tolower, w_tolower, 'A', 'Z')

/* ================================================================== */
/* ft_strlen                                                          */
/* ================================================================== */

static void	strlen_empty(void)
{
	CASE("ft_strlen(\"\") [guarded]");
	EXPECT(ft_strlen(t_gstr("")) == 0, "expected 0, got %zu", ft_strlen(t_gstr("")));
	CASE("ft_strlen(\"\") [front guarded]");
	EXPECT(ft_strlen(t_gstr_front("")) == 0, "expected 0");
}

static void	strlen_lengths(void)
{
	char	buf[400];
	size_t	n;
	size_t	got;

	n = 1;
	while (n <= 300)
	{
		memset(buf, 'a' + n % 26, n);
		buf[n] = 0;
		CASE("ft_strlen(<%zu chars>) [guarded]", n);
		got = ft_strlen(t_gstr(buf));
		EXPECT(got == n, "expected %zu, got %zu", n, got);
		CASE("ft_strlen(<%zu chars>) [front guarded]", n);
		got = ft_strlen(t_gstr_front(buf));
		EXPECT(got == n, "expected %zu, got %zu", n, got);
		n++;
	}
}

static void	strlen_high(void)
{
	const char	*s[] = {"\xff", "\x80", "\xff\x80\x01", "caf\xc3\xa9", "\x7f\x80\x81\xfe"};
	size_t		i;

	i = 0;
	while (i < sizeof(s) / sizeof(*s))
	{
		CASE("ft_strlen(\"%s\") [guarded]", t_esc(s[i]));
		EXPECT(ft_strlen(t_gstr(s[i])) == strlen(s[i]), "expected %zu, got %zu", strlen(s[i]),
			ft_strlen(t_gstr(s[i])));
		i++;
	}
}

static void	strlen_first_nul(void)
{
	char	*g;

	g = t_gmem("abc\0def", 8);
	CASE("ft_strlen(\"abc\\0def\") [guarded]");
	EXPECT(ft_strlen(g) == 3, "expected 3, got %zu", ft_strlen(g));
	g = t_gmem("\0abc", 5);
	CASE("ft_strlen(\"\\0abc\") [guarded]");
	EXPECT(ft_strlen(g) == 0, "expected 0, got %zu", ft_strlen(g));
}

static void	strlen_align(void)
{
	char	*g;
	size_t	off;
	size_t	len;

	off = 0;
	while (off < 16)
	{
		len = 0;
		while (len <= 40)
		{
			g = t_gmem(NULL, off + len + 1);
			memset(g, 'x', off + len);
			g[off + len] = 0;
			CASE("ft_strlen(<%zu chars starting at offset %zu>) [guarded]", len, off);
			EXPECT(ft_strlen(g + off) == len, "expected %zu, got %zu", len, ft_strlen(g + off));
			len++;
		}
		off++;
	}
}

static void	strlen_big(void)
{
	size_t	n;

	n = 64 << 20;
	CASE("ft_strlen(<64 MB string>) [guarded]");
	EXPECT(ft_strlen(big_gstr(n, 'x')) == n, "expected %zu", n);
}

static const t_test	g_ft_strlen[] = {
	TEST("empty string", "\"\" has length 0, and nothing after its '\\0' may be read (the next byte is a \
protected page).", strlen_empty),
	TEST("every length 1..300 [guarded]", "The count must stop exactly at the '\\0'. The string ends \
right before a protected page: reading one byte too far is a segfault.", strlen_lengths),
	TEST("bytes > 127", "Bytes like 0xff or UTF-8 'é' are ordinary chars, only '\\0' ends the string. A \
signed char compared with > 0 stops too early.", strlen_high),
	TEST("stops at the first '\\0'", "\"abc\\0def\" has length 3 and \"\\0abc\" length 0: only the first \
'\\0' counts.", strlen_first_nul),
	TEST("every alignment (word-at-a-time reads)", "Strings starting at every offset 0..15 and ending at a \
protected page: reading 8 bytes at a time past the '\\0' crashes.", strlen_align),
	TEST_SLOW("64 MB string", "A very long string: an int counter, or anything quadratic, fails or times \
out here.", strlen_big),
};

/* ================================================================== */
/* ft_memset / ft_bzero                                               */
/* ================================================================== */

static void	memset_ret(void)
{
	char	buf[16];
	char	*g;

	CASE("ft_memset(buf, 'x', 5)");
	EXPECT(ft_memset(buf, 'x', 5) == buf, "must return its first argument");
	g = t_gmem(NULL, 3);
	CASE("ft_memset(ptr, 0, 3) [guarded]");
	EXPECT(ft_memset(g, 0, 3) == g, "must return its first argument");
	CASE("ft_memset(buf, 'q', 0)");
	EXPECT(ft_memset(buf, 'q', 0) == buf, "must return its first argument even when n = 0");
}

static void	memset_sizes(void)
{
	unsigned char	*g;
	size_t			n;

	n = 0;
	while (n <= 130)
	{
		CASE("ft_memset(ptr, 'x', %zu) [guarded]", n);
		g = t_gmem(NULL, n);
		ft_memset(g, 'x', n);
		EXPECT(all_bytes(g, n, 'x'), "not every byte was set");
		CASE("ft_memset(ptr, 'x', %zu) [front guarded]", n);
		g = t_gmem_front(NULL, n);
		ft_memset(g, 'x', n);
		EXPECT(all_bytes(g, n, 'x'), "not every byte was set");
		n++;
	}
}

static void	memset_neighbours(void)
{
	unsigned char	buf[64];
	size_t			i;
	int				ok;

	memset(buf, 'Z', sizeof(buf));
	CASE("ft_memset(buf + 8, 'x', 10) in a 64-byte buffer of 'Z'");
	ft_memset(buf + 8, 'x', 10);
	ok = 1;
	i = 0;
	while (i < sizeof(buf))
	{
		ok &= (buf[i] == ((i >= 8 && i < 18) ? 'x' : 'Z'));
		i++;
	}
	EXPECT(ok, "wrote outside [8, 18): \"%s\"", t_escn((char *)buf, 24));
}

static void	memset_uchar(void)
{
	unsigned char	buf[8];
	int				cs[] = {256 + 'A', -1, 0x1ff, INT_MIN, -128, 128, 255, 0x12345678};
	size_t			i;

	i = 0;
	while (i < sizeof(cs) / sizeof(*cs))
	{
		memset(buf, 'Z', sizeof(buf));
		CASE("ft_memset(buf, %d, 5)", cs[i]);
		ft_memset(buf, cs[i], 5);
		EXPECT(all_bytes(buf, 5, (unsigned char)cs[i]) && buf[5] == 'Z',
			"every byte must be (unsigned char)%d = 0x%02x, got 0x%02x", cs[i], (unsigned char)cs[i], buf[0]);
		i++;
	}
}

static void	memset_zero(void)
{
	char	buf[4];

	CASE("ft_memset(<pointer right at a protected page>, 'x', 0)");
	ft_memset(t_gmem(NULL, 0), 'x', 0);
	CASE("ft_memset(buf, 'x', 0) leaves buf alone");
	memcpy(buf, "abc", 4);
	ft_memset(buf, 'x', 0);
	EXPECT(!strcmp(buf, "abc"), "buffer changed with n = 0");
}

static void	memset_align(void)
{
	unsigned char	a[96];
	unsigned char	b[96];
	size_t			off;
	size_t			n;

	off = 0;
	while (off < 16)
	{
		n = 0;
		while (n <= 64)
		{
			CASE("ft_memset(buf + %zu, 0xAB, %zu) (unaligned)", off, n);
			memset(a, 0, sizeof(a));
			memset(b, 0, sizeof(b));
			memset(b + off, 0xAB, n);
			ft_memset(a + off, 0xAB, n);
			EXPECT(!memcmp(a, b, sizeof(a)), "result differs from memset");
			n++;
		}
		off++;
	}
}

static void	memset_big(void)
{
	unsigned char	*g;
	size_t			n;

	n = 32 << 20;
	g = t_gmem(NULL, n);
	CASE("ft_memset(<32 MB>, 0x5A, 32 MB) [guarded]");
	ft_memset(g, 0x5A, n);
	EXPECT(all_bytes(g, n, 0x5A), "not every byte was set");
}

static const t_test	g_ft_memset[] = {
	TEST("returns its first argument", "memset returns s, whatever n is (also 0).", memset_ret),
	TEST("n = 0..130 [guarded, front guarded]", "Exactly n bytes must be written: the buffer is placed \
against a protected page, one byte too many (before or after) is a segfault.", memset_sizes),
	TEST("does not touch the bytes around", "Only [s, s + n) may change: the bytes before and after keep \
their value.", memset_neighbours),
	TEST("c converted to unsigned char", "c is an int but each byte gets (unsigned char)c: 256 + 'A' \
writes 'A', -1 writes 0xff, INT_MIN writes 0.", memset_uchar),
	TEST("n = 0 writes nothing", "With n = 0 nothing is written, even when s points right at a protected \
page.", memset_zero),
	TEST("unaligned start and length", "Every offset 0..15 and length 0..64 compared with the libc: \
implementations writing words (8 bytes) at a time often get the tail wrong.", memset_align),
	TEST_SLOW("32 MB", "A big buffer, set completely: an int n or a slow loop fails here.", memset_big),
};

static void	bzero_sizes(void)
{
	unsigned char	*g;
	size_t			n;

	n = 0;
	while (n <= 130)
	{
		CASE("ft_bzero(ptr, %zu) [guarded]", n);
		g = t_gmem(NULL, n);
		memset(g, 0xFF, n);
		ft_bzero(g, n);
		EXPECT(all_bytes(g, n, 0), "not every byte was zeroed");
		n++;
	}
}

static void	bzero_front(void)
{
	unsigned char	*g;
	size_t			n;

	n = 0;
	while (n <= 130)
	{
		CASE("ft_bzero(ptr, %zu) [front guarded]", n);
		g = t_gmem_front(NULL, n);
		memset(g, 0xFF, n);
		ft_bzero(g, n);
		EXPECT(all_bytes(g, n, 0), "not every byte was zeroed");
		n++;
	}
}

static void	bzero_neighbours(void)
{
	unsigned char	buf[32];

	memset(buf, 'Z', sizeof(buf));
	CASE("ft_bzero(buf + 4, 3) in a buffer of 'Z'");
	ft_bzero(buf + 4, 3);
	EXPECT(buf[3] == 'Z' && buf[4] == 0 && buf[5] == 0 && buf[6] == 0 && buf[7] == 'Z',
		"wrong bytes: \"%s\"", t_escn((char *)buf, 10));
}

static void	bzero_zero(void)
{
	char	buf[4];

	CASE("ft_bzero(<pointer right at a protected page>, 0)");
	ft_bzero(t_gmem(NULL, 0), 0);
	CASE("ft_bzero(buf, 0) leaves buf alone");
	memcpy(buf, "abc", 4);
	ft_bzero(buf, 0);
	EXPECT(!strcmp(buf, "abc"), "buffer changed with n = 0");
}

static void	bzero_align(void)
{
	unsigned char	a[96];
	unsigned char	b[96];
	size_t			off;
	size_t			n;

	off = 0;
	while (off < 16)
	{
		n = 0;
		while (n <= 64)
		{
			CASE("ft_bzero(buf + %zu, %zu) (unaligned)", off, n);
			memset(a, 0xEE, sizeof(a));
			memset(b, 0xEE, sizeof(b));
			memset(b + off, 0, n);
			ft_bzero(a + off, n);
			EXPECT(!memcmp(a, b, sizeof(a)), "result differs from memset(s, 0, n)");
			n++;
		}
		off++;
	}
}

static void	bzero_big(void)
{
	unsigned char	*g;
	size_t			n;

	n = 32 << 20;
	g = t_gmem(NULL, n);
	memset(g, 0x77, n);
	CASE("ft_bzero(<32 MB>) [guarded]");
	ft_bzero(g, n);
	EXPECT(all_bytes(g, n, 0), "not every byte was zeroed");
}

static const t_test	g_ft_bzero[] = {
	TEST("n = 0..130 [guarded]", "Exactly n bytes set to 0: the buffer ends at a protected page, one \
byte too many is a segfault.", bzero_sizes),
	TEST("n = 0..130 [front guarded]", "Same, with the protected page right before the buffer: writing \
s[-1] is a segfault.", bzero_front),
	TEST("does not touch the bytes around", "Only [s, s + n) may change.", bzero_neighbours),
	TEST("n = 0 writes nothing", "With n = 0 nothing is written, even when s points at a protected \
page.", bzero_zero),
	TEST("unaligned start and length", "Every offset 0..15 and length 0..64 compared with the libc.", bzero_align),
	TEST_SLOW("32 MB", "A big buffer zeroed completely: an int n or a slow loop fails here.", bzero_big),
};

/* ================================================================== */
/* ft_memcpy / ft_memmove                                             */
/* ================================================================== */

static void	memcpy_ret(void)
{
	char	a[8];

	CASE("ft_memcpy(a, \"abc\", 3)");
	EXPECT(ft_memcpy(a, "abc", 3) == a, "must return dst");
	CASE("ft_memcpy(a, \"abc\", 0)");
	EXPECT(ft_memcpy(a, "abc", 0) == a, "must return dst even when n = 0");
}

static void	memcpy_sizes(void)
{
	unsigned char	pat[256];
	unsigned char	*src;
	unsigned char	*dst;
	size_t			n;

	fill_pattern(pat, sizeof(pat), 3);
	n = 0;
	while (n <= 130)
	{
		CASE("ft_memcpy(dst, src, %zu) [both guarded]", n);
		src = t_gmem(pat, n);
		dst = t_gmem(NULL, n);
		ft_memcpy(dst, src, n);
		EXPECT(memcmp(dst, pat, n) == 0, "copied bytes differ");
		n++;
	}
}

static void	memcpy_front(void)
{
	unsigned char	pat[256];
	unsigned char	*src;
	unsigned char	*dst;
	size_t			n;

	fill_pattern(pat, sizeof(pat), 5);
	n = 0;
	while (n <= 130)
	{
		CASE("ft_memcpy(dst, src, %zu) [both front guarded]", n);
		src = t_gmem_front(pat, n);
		dst = t_gmem_front(NULL, n);
		ft_memcpy(dst, src, n);
		EXPECT(memcmp(dst, pat, n) == 0, "copied bytes differ");
		n++;
	}
}

static void	memcpy_nul(void)
{
	unsigned char	a[8];

	CASE("ft_memcpy(dst, \"ab\\0cd\", 5)");
	memset(a, 'Z', 8);
	ft_memcpy(a, "ab\0cd", 5);
	EXPECT(memcmp(a, "ab\0cdZ", 6) == 0, "got \"%s\" (it must not stop at '\\0')", t_escn((char *)a, 6));
	CASE("ft_memcpy(dst, \"\\0\\0\\xff\", 3)");
	ft_memcpy(a, "\0\0\xff", 3);
	EXPECT(memcmp(a, "\0\0\xff", 3) == 0, "got \"%s\"", t_escn((char *)a, 3));
}

static void	memcpy_zero(void)
{
	char	buf[4];

	CASE("ft_memcpy(<protected page>, <protected page>, 0)");
	ft_memcpy(t_gmem(NULL, 0), t_gmem(NULL, 0), 0);
	CASE("ft_memcpy(buf, \"xyz\", 0) leaves buf alone");
	memcpy(buf, "abc", 4);
	ft_memcpy(buf, "xyz", 0);
	EXPECT(!strcmp(buf, "abc"), "buffer changed with n = 0");
}

static void	memcpy_align(void)
{
	unsigned char	pat[256];
	unsigned char	a[128];
	unsigned char	b[128];
	size_t			off;
	size_t			so;
	size_t			n;

	fill_pattern(pat, sizeof(pat), 9);
	off = 0;
	while (off < 16)
	{
		so = 0;
		while (so < 8)
		{
			n = 0;
			while (n <= 80)
			{
				CASE("ft_memcpy(a + %zu, pattern + %zu, %zu) (unaligned)", off, so, n);
				memset(a, 0xEE, sizeof(a));
				memset(b, 0xEE, sizeof(b));
				memcpy(b + off, pat + so, n);
				ft_memcpy(a + off, pat + so, n);
				EXPECT(memcmp(a, b, sizeof(a)) == 0, "result differs from memcpy");
				n++;
			}
			so++;
		}
		off++;
	}
}

static void	memcpy_big(void)
{
	unsigned char	*src;
	unsigned char	*dst;
	size_t			n;

	n = 16 << 20;
	src = t_gmem(NULL, n);
	dst = t_gmem(NULL, n);
	fill_pattern(src, n, 11);
	CASE("ft_memcpy(<16 MB>) [guarded]");
	ft_memcpy(dst, src, n);
	EXPECT(memcmp(dst, src, n) == 0, "copied bytes differ");
}

static void	memcpy_null(void)
{
	CASE("ft_memcpy(NULL, NULL, 0)");
	ft_memcpy(NULL, NULL, 0);
	CASE("ft_memcpy(NULL, NULL, 5)");
	EXPECT(ft_memcpy(NULL, NULL, 5) == NULL, "should return NULL (like the glibc and macOS)");
}

static const t_test	g_ft_memcpy[] = {
	TEST("returns dst", "memcpy returns dst, also when n = 0.", memcpy_ret),
	TEST("n = 0..130 [both guarded]", "Exactly n bytes are read and written: both buffers end at a \
protected page.", memcpy_sizes),
	TEST("n = 0..130 [both front guarded]", "Same with the protected page before the buffers: copying \
backwards past the start crashes.", memcpy_front),
	TEST("does not stop at '\\0'", "memcpy copies bytes, not a string: '\\0' is an ordinary byte.", memcpy_nul),
	TEST("n = 0 copies nothing", "With n = 0 nothing is read or written, even at a protected page.", memcpy_zero),
	TEST("unaligned src, dst and length", "Every dst offset 0..15, src offset 0..7 and length 0..80 \
compared with the libc: word-at-a-time copies often break the tail.", memcpy_align),
	TEST_SLOW("16 MB", "A big copy: an int n or a slow loop fails here.", memcpy_big),
	TEST_UB("NULL, NULL", "memcpy(NULL, NULL, n) is undefined, but evaluators try it: the libc on 42 \
machines returns NULL without crashing.", memcpy_null),
};

static void	memmove_ret(void)
{
	char	a[16];

	memcpy(a, "0123456789", 11);
	CASE("ft_memmove(a + 2, a, 5)");
	EXPECT(ft_memmove(a + 2, a, 5) == a + 2, "must return dst");
	CASE("ft_memmove(a, a + 2, 5)");
	EXPECT(ft_memmove(a, a + 2, 5) == a, "must return dst");
	CASE("ft_memmove(a, a, 0)");
	EXPECT(ft_memmove(a, a, 0) == a, "must return dst");
}

static void	memmove_separate(void)
{
	unsigned char	pat[128];
	unsigned char	*src;
	unsigned char	*dst;
	size_t			n;

	fill_pattern(pat, sizeof(pat), 1);
	n = 0;
	while (n <= 100)
	{
		CASE("ft_memmove(dst, src, %zu) [separate guarded buffers]", n);
		src = t_gmem(pat, n);
		dst = t_gmem(NULL, n);
		ft_memmove(dst, src, n);
		EXPECT(memcmp(dst, pat, n) == 0, "copied bytes differ");
		CASE("ft_memmove(dst, src, %zu) [separate front guarded buffers]", n);
		src = t_gmem_front(pat, n);
		dst = t_gmem_front(NULL, n);
		ft_memmove(dst, src, n);
		EXPECT(memcmp(dst, pat, n) == 0, "copied bytes differ");
		n++;
	}
}

static void	memmove_one(size_t d, size_t s, size_t n)
{
	unsigned char	a[256];
	unsigned char	b[256];

	fill_pattern(a, sizeof(a), 5);
	fill_pattern(b, sizeof(b), 5);
	CASE("ft_memmove(buf + %zu, buf + %zu, %zu)", d, s, n);
	memmove(b + d, b + s, n);
	ft_memmove(a + d, a + s, n);
	EXPECT(memcmp(a, b, sizeof(a)) == 0, "result differs from memmove");
}

static void	memmove_fwd(void)
{
	memmove_one(1, 0, 10);
	memmove_one(3, 0, 20);
	memmove_one(8, 0, 64);
	memmove_one(1, 0, 200);
	memmove_one(100, 3, 150);
}

static void	memmove_bwd(void)
{
	memmove_one(0, 1, 10);
	memmove_one(0, 3, 20);
	memmove_one(0, 8, 64);
	memmove_one(0, 1, 200);
	memmove_one(3, 100, 150);
}

static void	memmove_same(void)
{
	unsigned char	a[16];
	unsigned char	b[16];

	fill_pattern(a, 16, 9);
	fill_pattern(b, 16, 9);
	CASE("ft_memmove(buf, buf, 10)");
	EXPECT(ft_memmove(a, a, 10) == a && memcmp(a, b, 16) == 0, "buffer changed or wrong return");
}

static void	memmove_all(void)
{
	size_t	s;
	size_t	d;
	size_t	n;

	s = 0;
	while (s <= 24)
	{
		d = 0;
		while (d <= 24)
		{
			n = 0;
			while (n <= 48)
				memmove_one(d, s, n++);
			d++;
		}
		s++;
	}
}

static void	memmove_guard(void)
{
	unsigned char	*g;
	unsigned char	*ref;
	size_t			n;

	n = 0;
	while (n <= 64)
	{
		CASE("ft_memmove(g + 1, g, %zu) [guarded, the copy ends at the guard]", n);
		g = t_gmem(NULL, n + 1);
		fill_pattern(g, n + 1, 1);
		ref = t_gmem(g, n + 1);
		memmove(ref + 1, ref, n);
		ft_memmove(g + 1, g, n);
		EXPECT(memcmp(g, ref, n + 1) == 0, "result differs from memmove");
		CASE("ft_memmove(g, g + 1, %zu) [front guarded, the copy starts at the guard]", n);
		g = t_gmem_front(NULL, n + 1);
		fill_pattern(g, n + 1, 1);
		ref = t_gmem(g, n + 1);
		memmove(ref, ref + 1, n);
		ft_memmove(g, g + 1, n);
		EXPECT(memcmp(g, ref, n + 1) == 0, "result differs from memmove");
		n++;
	}
}

static void	memmove_big(void)
{
	unsigned char	*g;
	unsigned char	*ref;
	size_t			n;

	n = 16 << 20;
	g = t_gmem(NULL, n);
	ref = t_gmem(NULL, n);
	fill_pattern(g, n, 3);
	fill_pattern(ref, n, 3);
	CASE("ft_memmove(big + 1, big, 16 MB - 1) [guarded]");
	memmove(ref + 1, ref, n - 1);
	ft_memmove(g + 1, g, n - 1);
	EXPECT(memcmp(g, ref, n) == 0, "result differs from memmove");
	CASE("ft_memmove(big, big + 1, 16 MB - 1) [guarded]");
	memmove(ref, ref + 1, n - 1);
	ft_memmove(g, g + 1, n - 1);
	EXPECT(memcmp(g, ref, n) == 0, "result differs from memmove");
}

static void	memmove_null(void)
{
	CASE("ft_memmove(NULL, NULL, 0)");
	ft_memmove(NULL, NULL, 0);
	CASE("ft_memmove(NULL, NULL, 5)");
	EXPECT(ft_memmove(NULL, NULL, 5) == NULL, "should return NULL (like the glibc and macOS)");
}

static const t_test	g_ft_memmove[] = {
	TEST("returns dst", "memmove returns dst in every direction, also when n = 0.", memmove_ret),
	TEST("no overlap [guarded, front guarded]", "Separate buffers against protected pages: exactly n \
bytes read and written.", memmove_separate),
	TEST("overlap, dst after src", "dst inside [src, src + n): copying from the start would overwrite \
bytes before reading them, it must copy backwards.", memmove_fwd),
	TEST("overlap, dst before src", "src inside [dst, dst + n): it must copy forwards (copying backwards \
here is just as wrong).", memmove_bwd),
	TEST("dst == src", "Moving a buffer onto itself leaves it unchanged.", memmove_same),
	TEST("every overlap (offsets 0..24, n 0..48)", "Every combination of src / dst offsets and length \
compared with the libc memmove.", memmove_all),
	TEST("overlap touching a protected page", "The overlapping copy ends (or starts) exactly at a \
protected page: reading one byte too many is a segfault.", memmove_guard),
	TEST_SLOW("16 MB overlapping, both directions", "A big overlapping move, forwards and backwards.", memmove_big),
	TEST_UB("NULL, NULL", "memmove(NULL, NULL, n) is undefined, but evaluators try it: the libc returns \
NULL without crashing.", memmove_null),
};

/* ================================================================== */
/* ft_strlcpy / ft_strlcat                                            */
/* ================================================================== */

static const char	*g_srcs[] = {"", "a", "hello", "hello world 42 !", "\xff\x80\x01",
	"lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod"};

#define NSRCS (sizeof(g_srcs) / sizeof(*g_srcs))

static void	strlcpy_ret(void)
{
	char	dst[128];
	size_t	i;
	size_t	sizes[] = {0, 1, 2, 5, 100};
	size_t	k;
	size_t	r;

	i = 0;
	while (i < NSRCS)
	{
		k = 0;
		while (k < sizeof(sizes) / sizeof(*sizes))
		{
			CASE("ft_strlcpy(dst[128], \"%s\", %zu)", t_esc(g_srcs[i]), sizes[k]);
			r = ft_strlcpy(dst, t_gstr(g_srcs[i]), sizes[k]);
			EXPECT(r == strlen(g_srcs[i]), "must return strlen(src) = %zu, got %zu", strlen(g_srcs[i]), r);
			k++;
		}
		i++;
	}
}

static void	strlcpy_size0(void)
{
	size_t	i;
	size_t	r;

	i = 0;
	while (i < NSRCS)
	{
		CASE("ft_strlcpy(<protected page>, \"%s\", 0)", t_esc(g_srcs[i]));
		r = ft_strlcpy(t_gmem(NULL, 0), t_gstr(g_srcs[i]), 0);
		EXPECT(r == strlen(g_srcs[i]), "must return %zu, got %zu", strlen(g_srcs[i]), r);
		i++;
	}
}

static void	strlcpy_size1(void)
{
	char	*dst;
	size_t	i;

	i = 0;
	while (i < NSRCS)
	{
		CASE("ft_strlcpy(dst[1], \"%s\", 1) [guarded]", t_esc(g_srcs[i]));
		dst = t_gmem("Z", 1);
		ft_strlcpy(dst, t_gstr(g_srcs[i]), 1);
		EXPECT(dst[0] == 0, "dst must become \"\" (only room for the '\\0'), got 0x%02x", (unsigned char)dst[0]);
		i++;
	}
}

static void	strlcpy_all(void)
{
	char	ref[128];
	char	*dst;
	char	*src;
	size_t	i;
	size_t	size;
	size_t	len;

	i = 0;
	while (i < NSRCS)
	{
		len = strlen(g_srcs[i]);
		src = t_gstr(g_srcs[i]);
		size = 0;
		while (size <= len + 3)
		{
			CASE("ft_strlcpy(dst[%zu], \"%s\", %zu) [guarded]", size, t_esc(g_srcs[i]), size);
			dst = t_gmem(NULL, size);
			memset(dst, 0xAA, size);
			memset(ref, 0xAA, sizeof(ref));
			ref_strlcpy(ref, g_srcs[i], size);
			ft_strlcpy(dst, src, size);
			EXPECT(memcmp(dst, ref, size) == 0, "expected \"%s\", got \"%s\"",
				t_escn(ref, size), t_escn(dst, size));
			size++;
		}
		i++;
	}
}

static void	strlcpy_tail(void)
{
	char	dst[64];
	size_t	i;
	size_t	len;

	i = 0;
	while (i < 5)
	{
		len = strlen(g_srcs[i]);
		memset(dst, 0xAA, sizeof(dst));
		CASE("ft_strlcpy(dst[64], \"%s\", 64)", t_esc(g_srcs[i]));
		ft_strlcpy(dst, g_srcs[i], sizeof(dst));
		EXPECT(!strcmp(dst, g_srcs[i]), "wrong copy \"%s\"", t_esc(dst));
		EXPECT(all_bytes((unsigned char *)dst + len + 1, sizeof(dst) - len - 1, 0xAA),
			"wrote after the '\\0' (only strlen(src) + 1 bytes may be written)");
		i++;
	}
}

static void	strlcpy_srcguard(void)
{
	char	dst[128];
	size_t	i;

	i = 0;
	while (i < NSRCS)
	{
		CASE("ft_strlcpy(dst[128], \"%s\", 128) [src guarded]", t_esc(g_srcs[i]));
		ft_strlcpy(dst, t_gstr(g_srcs[i]), sizeof(dst));
		CASE("ft_strlcpy(dst[128], \"%s\", 128) [src front guarded]", t_esc(g_srcs[i]));
		ft_strlcpy(dst, t_gstr_front(g_srcs[i]), sizeof(dst));
		EXPECT(!strcmp(dst, g_srcs[i]), "wrong copy");
		i++;
	}
}

static void	strlcpy_sizemax(void)
{
	char	*dst;
	size_t	i;
	size_t	r;

	i = 0;
	while (i < NSRCS)
	{
		CASE("ft_strlcpy(dst[%zu], \"%s\", SIZE_MAX) [guarded]", strlen(g_srcs[i]) + 1, t_esc(g_srcs[i]));
		dst = t_gmem(NULL, strlen(g_srcs[i]) + 1);
		r = ft_strlcpy(dst, t_gstr(g_srcs[i]), SIZE_MAX);
		EXPECT(r == strlen(g_srcs[i]) && !strcmp(dst, g_srcs[i]), "wrong copy or return (%zu)", r);
		i++;
	}
}

static void	strlcpy_long(void)
{
	char	*dst;
	size_t	r;

	dst = t_gmem(NULL, 10);
	CASE("ft_strlcpy(dst[10], <4 MB string>, 10) [guarded]");
	r = ft_strlcpy(dst, big_gstr(4 << 20, 'L'), 10);
	EXPECT(r == (size_t)4 << 20, "must return 4194304, got %zu", r);
	EXPECT(!memcmp(dst, "LLLLLLLLL", 10), "dst must be 9 'L' + '\\0'");
}

static void	strlcpy_null(void)
{
	CASE("ft_strlcpy(NULL, \"abc\", 0)");
	EXPECT(ft_strlcpy(NULL, "abc", 0) == 3, "should return 3 (nothing is written when size is 0)");
}

static const t_test	g_ft_strlcpy[] = {
	TEST("returns strlen(src)", "strlcpy always returns the length of src, whatever size is: that is \
how the caller detects truncation.", strlcpy_ret),
	TEST("size 0 writes nothing", "With size 0 not a single byte of dst is touched (dst points at a \
protected page here), and strlen(src) is still returned.", strlcpy_size0),
	TEST("size 1: dst becomes \"\"", "With size 1 there is only room for the '\\0'.", strlcpy_size1),
	TEST("truncation at every size [guarded]", "For every size from 0 to strlen(src) + 3, dst is exactly \
size bytes before a protected page: at most size - 1 chars + '\\0', compared with BSD.", strlcpy_all),
	TEST("nothing written after the '\\0'", "Only strlen(src) + 1 bytes may be written when src fits: \
filling the rest of dst is wrong.", strlcpy_tail),
	TEST("src never read past its '\\0'", "src ends (or starts) at a protected page.", strlcpy_srcguard),
	TEST("size = SIZE_MAX", "A huge size must not overflow (size - 1, len + 1...): copy src entirely.", strlcpy_sizemax),
	TEST("4 MB src into 10 bytes", "The return value is strlen(src) even when almost nothing is copied.", strlcpy_long),
	TEST_UB("NULL dst with size 0", "Nothing may be written with size 0, so dst NULL works with BSD: \
evaluators try it.", strlcpy_null),
};

static void	strlcat_basic(void)
{
	char	buf[64];
	size_t	r;

	strcpy(buf, "Hello");
	CASE("ft_strlcat(\"Hello\", \" World\", 64)");
	r = ft_strlcat(buf, t_gstr(" World"), sizeof(buf));
	EXPECT(r == 11 && !strcmp(buf, "Hello World"), "expected \"Hello World\" / 11, got \"%s\" / %zu", t_esc(buf), r);
	CASE("ft_strlcat(\"Hello World\", \"!\", 64)");
	r = ft_strlcat(buf, "!", sizeof(buf));
	EXPECT(r == 12 && !strcmp(buf, "Hello World!"), "expected \"Hello World!\" / 12, got \"%s\" / %zu", t_esc(buf), r);
}

static void	strlcat_size0(void)
{
	size_t	r;

	CASE("ft_strlcat(<protected page>, \"abc\", 0)");
	r = ft_strlcat(t_gmem(NULL, 0), t_gstr("abc"), 0);
	EXPECT(r == 3, "must return strlen(src) = 3, got %zu", r);
	CASE("ft_strlcat(<protected page>, \"\", 0)");
	r = ft_strlcat(t_gmem(NULL, 0), t_gstr(""), 0);
	EXPECT(r == 0, "must return 0, got %zu", r);
}

static void	strlcat_nonul(void)
{
	char	*dst;
	size_t	size;
	size_t	r;

	size = 1;
	while (size <= 5)
	{
		CASE("ft_strlcat(dst[%zu] = \"hello\" without '\\0' inside size, \"xyz\", %zu) [guarded]", size, size);
		dst = t_gmem("hello", size);
		r = ft_strlcat(dst, t_gstr("xyz"), size);
		EXPECT(r == size + 3, "must return size + strlen(src) = %zu, got %zu", size + 3, r);
		EXPECT(!memcmp(dst, "hello", size), "dst must not change when it has no '\\0' inside size");
		size++;
	}
}

static void	strlcat_exact(void)
{
	char	*dst;
	size_t	r;

	CASE("ft_strlcat(dst[6] = \"hello\", \"xyz\", 6) [guarded]");
	dst = t_gmem("hello", 6);
	r = ft_strlcat(dst, t_gstr("xyz"), 6);
	EXPECT(r == 8 && !memcmp(dst, "hello", 6), "no room: nothing appended, returns 8 (got %zu, \"%s\")", r, t_esc(dst));
	CASE("ft_strlcat(dst[7] = \"hello\", \"xyz\", 7) [guarded]");
	dst = t_gmem("hello\0Z", 7);
	r = ft_strlcat(dst, t_gstr("xyz"), 7);
	EXPECT(r == 8 && !memcmp(dst, "hellox", 7), "expected \"hellox\" / 8, got \"%s\" / %zu", t_escn(dst, 7), r);
}

static void	strlcat_all(void)
{
	const char	*dsts[] = {"", "ab", "hello", "lorem ipsum"};
	const char	*srcs[] = {"", "x", "world!!", "dolor sit amet"};
	char		ref[128];
	char		*dst;
	size_t		i;
	size_t		j;
	size_t		size;
	size_t		exp;
	size_t		r;

	i = 0;
	while (i < 4)
	{
		j = 0;
		while (j < 4)
		{
			size = 0;
			while (size <= strlen(dsts[i]) + strlen(srcs[j]) + 3)
			{
				CASE("ft_strlcat(dst[%zu] = \"%s\", \"%s\", %zu) [guarded]", size,
					t_esc(dsts[i]), t_esc(srcs[j]), size);
				memset(ref, 0xAA, sizeof(ref));
				memcpy(ref, dsts[i], strlen(dsts[i]) + 1 < size ? strlen(dsts[i]) + 1 : size);
				dst = t_gmem(ref, size);
				exp = ref_strlcat(ref, srcs[j], size);
				r = ft_strlcat(dst, t_gstr(srcs[j]), size);
				EXPECT(r == exp, "expected return %zu, got %zu", exp, r);
				EXPECT(memcmp(dst, ref, size) == 0, "expected \"%s\", got \"%s\"",
					t_escn(ref, size), t_escn(dst, size));
				size++;
			}
			j++;
		}
		i++;
	}
}

static void	strlcat_empty(void)
{
	char	buf[16];
	size_t	r;

	buf[0] = 0;
	CASE("ft_strlcat(\"\", \"abc\", 16)");
	r = ft_strlcat(buf, "abc", sizeof(buf));
	EXPECT(r == 3 && !strcmp(buf, "abc"), "expected \"abc\" / 3, got \"%s\" / %zu", t_esc(buf), r);
	CASE("ft_strlcat(\"abc\", \"\", 16)");
	r = ft_strlcat(buf, t_gstr(""), sizeof(buf));
	EXPECT(r == 3 && !strcmp(buf, "abc"), "expected \"abc\" / 3, got \"%s\" / %zu", t_esc(buf), r);
	buf[0] = 0;
	CASE("ft_strlcat(\"\", \"\", 1)");
	r = ft_strlcat(buf, "", 1);
	EXPECT(r == 0 && !buf[0], "expected \"\" / 0, got %zu", r);
}

static void	strlcat_tail(void)
{
	char	buf[32];

	memset(buf, 0xAA, sizeof(buf));
	memcpy(buf, "ab", 3);
	CASE("ft_strlcat(\"ab\" in a 32-byte buffer, \"cd\", 32)");
	ft_strlcat(buf, t_gstr_front("cd"), sizeof(buf));
	EXPECT(!strcmp(buf, "abcd"), "expected \"abcd\", got \"%s\"", t_esc(buf));
	EXPECT(all_bytes((unsigned char *)buf + 5, sizeof(buf) - 5, 0xAA),
		"wrote after the new '\\0'");
}

static void	strlcat_null(void)
{
	CASE("ft_strlcat(NULL, \"abc\", 0)");
	EXPECT(ft_strlcat(NULL, "abc", 0) == 3, "should return 3 (BSD reads nothing from dst when size is 0)");
}

static const t_test	g_ft_strlcat[] = {
	TEST("appends and returns strlen(dst) + strlen(src)", "The normal case: dst has room, src is appended \
and the total length is returned.", strlcat_basic),
	TEST("size 0: dst is not even read", "With size 0, dst is neither read nor written (it points at a \
protected page here) and strlen(src) is returned. Calling strlen(dst) first crashes.", strlcat_size0),
	TEST("no '\\0' in dst within size", "When dst has no '\\0' in its first size bytes, strlcat returns \
size + strlen(src) and reads nothing past size (protected page there).", strlcat_nonul),
	TEST("size = strlen(dst) + 1 / + 2", "No room at all: nothing appended. One byte of room: one char \
+ '\\0'.", strlcat_exact),
	TEST("truncation at every size [guarded]", "Every dst / src / size combination compared with BSD, dst \
exactly size bytes before a protected page.", strlcat_all),
	TEST("empty dst, empty src", "Appending to \"\" or appending \"\" still returns the right total.", strlcat_empty),
	TEST("nothing written after the new '\\0'", "Only the appended chars and one '\\0' may be written.", strlcat_tail),
	TEST_UB("NULL dst with size 0", "With size 0 nothing is read from dst, so BSD accepts NULL: evaluators \
try it.", strlcat_null),
};

/* ================================================================== */
/* ft_strchr / ft_strrchr                                             */
/* ================================================================== */

static char	*w_strchr(const char *s, int c) { return ((char *)strchr(s, c)); }
static char	*w_strrchr(const char *s, int c) { return ((char *)strrchr(s, c)); }

static void	chr_one(const char *name, char *(*ft)(const char *, int),
	char *(*ref)(const char *, int), const char *s, int c, int front)
{
	char	*g;
	char	*got;
	char	*exp;

	g = front ? t_gstr_front(s) : t_gstr(s);
	CASE("%s(\"%s\", %d) [%s]", name, t_esc(s), c, front ? "front guarded" : "guarded");
	got = ft(g, c);
	exp = ref(s, c);
	if (!exp)
		EXPECT(got == NULL, "expected NULL, got a pointer to index %td", got - g);
	else
		EXPECT(got == g + (exp - s), "expected a pointer to index %td, got %s%td", exp - s,
			got ? "index " : "NULL ", got ? got - g : (ptrdiff_t)0);
}

/* every char of s (+ one absent) as c, c - 256 and c + 256 */
static void	chr_all(const char *name, char *(*ft)(const char *, int),
	char *(*ref)(const char *, int), const char *s)
{
	size_t	i;
	int		c;

	i = 0;
	while (i <= strlen(s) + 1)
	{
		c = i <= strlen(s) ? (unsigned char)s[i] : '~';
		chr_one(name, ft, ref, s, c, 0);
		chr_one(name, ft, ref, s, c, 1);
		chr_one(name, ft, ref, s, (char)c, 0);
		chr_one(name, ft, ref, s, c + 256, 0);
		chr_one(name, ft, ref, s, c - 256, 1);
		i++;
	}
}

#define CHR_TESTS(FN, W) \
static void	FN##_found(void) \
{ \
	chr_one(#FN, FN, W, "Hello, World!", 'o', 0); \
	chr_one(#FN, FN, W, "Hello, World!", 'H', 1); \
	chr_one(#FN, FN, W, "Hello, World!", '!', 0); \
	chr_one(#FN, FN, W, "abcabc", 'b', 0); \
	chr_one(#FN, FN, W, "aaaa", 'a', 1); \
} \
static void	FN##_nul(void) \
{ \
	chr_one(#FN, FN, W, "Hello", 0, 0); \
	chr_one(#FN, FN, W, "", 0, 0); \
	chr_one(#FN, FN, W, "", 0, 1); \
	chr_one(#FN, FN, W, "abc", 256, 0); \
} \
static void	FN##_absent(void) \
{ \
	chr_one(#FN, FN, W, "Hello", 'z', 0); \
	chr_one(#FN, FN, W, "Hello", 'z', 1); \
	chr_one(#FN, FN, W, "", 'a', 0); \
	chr_one(#FN, FN, W, "", 'a', 1); \
	chr_one(#FN, FN, W, "abc", 'A', 0); \
} \
static void	FN##_conv(void) \
{ \
	chr_one(#FN, FN, W, "tripouille", 't' + 256, 0); \
	chr_one(#FN, FN, W, "tripouille", 'e' + 512, 1); \
	chr_one(#FN, FN, W, "tripouille", 'l' - 256, 0); \
	chr_one(#FN, FN, W, "abc\xc8" "def", 0xc8 + 256, 0); \
	chr_one(#FN, FN, W, "abc", 'a' + 1024, 0); \
} \
static void	FN##_high(void) \
{ \
	chr_one(#FN, FN, W, "abc\xc8" "def\xc8", 0xc8, 0); \
	chr_one(#FN, FN, W, "abc\xc8" "def\xc8", -56, 1); \
	chr_one(#FN, FN, W, "\xff\x01\x7f\xff", 255, 0); \
	chr_one(#FN, FN, W, "\xff\x01\x7f\xff", -1, 1); \
	chr_one(#FN, FN, W, "\x80", 0x80, 1); \
} \
static void	FN##_guard(void) \
{ \
	chr_all(#FN, FN, W, ""); \
	chr_all(#FN, FN, W, "a"); \
	chr_all(#FN, FN, W, "Hello, World!"); \
	chr_all(#FN, FN, W, "abcabc"); \
	chr_all(#FN, FN, W, "tripouille\xc8\x80 end"); \
	chr_all(#FN, FN, W, "\xff\x01\x7f"); \
} \
static void	FN##_big(void) \
{ \
	char	*s; \
	size_t	n; \
\
	n = 8 << 20; \
	s = big_gstr(n, 'a'); \
	s[0] = 'X'; \
	s[n - 1] = 'Y'; \
	CASE(#FN "(<8 MB>, 'X') [guarded]"); \
	EXPECT(FN(s, 'X') == W(s, 'X'), "wrong pointer"); \
	CASE(#FN "(<8 MB>, 'Y') [guarded]"); \
	EXPECT(FN(s, 'Y') == W(s, 'Y'), "wrong pointer"); \
	CASE(#FN "(<8 MB>, 'Z') [guarded]"); \
	EXPECT(FN(s, 'Z') == NULL, "expected NULL"); \
}

CHR_TESTS(ft_strchr, w_strchr)
CHR_TESTS(ft_strrchr, w_strrchr)

static const t_test	g_ft_strchr[] = {
	TEST("finds the FIRST occurrence", "strchr returns a pointer to the first c in s (a pointer into s, \
not a copy).", ft_strchr_found),
	TEST("c = '\\0' finds the terminator", "The '\\0' is part of the string: strchr(s, '\\0') returns a \
pointer to it, not NULL. Same for c = 256 (converted to char, it is '\\0').", ft_strchr_nul),
	TEST("not found returns NULL", "When c is not in s, NULL (also for \"\").", ft_strchr_absent),
	TEST("c converted to char (c + 256)", "c is an int converted to char: strchr(s, 't' + 256) finds \
't'. Comparing *s == c directly never matches.", ft_strchr_conv),
	TEST("chars > 127", "Bytes like 0xc8 must be found with c = 200 and with c = -56. Mixing signed / \
unsigned comparisons breaks one of them.", ft_strchr_high),
	TEST("every char, never past the '\\0' [guarded]", "Every char of several strings (and one absent), \
the string ending / starting at a protected page.", ft_strchr_guard),
	TEST("8 MB string", "First / last / absent char of a big string.", ft_strchr_big),
};

static const t_test	g_ft_strrchr[] = {
	TEST("finds the LAST occurrence", "strrchr returns a pointer to the last c in s.", ft_strrchr_found),
	TEST("c = '\\0' finds the terminator", "strrchr(s, '\\0') returns a pointer to the '\\0', not NULL.",
		ft_strrchr_nul),
	TEST("not found returns NULL", "When c is not in s, NULL (also for \"\").", ft_strrchr_absent),
	TEST("c converted to char (c + 256)", "c is an int converted to char: strrchr(s, 'e' + 512) finds 'e'.",
		ft_strrchr_conv),
	TEST("chars > 127", "Bytes like 0xc8 must be found with c = 200 and with c = -56.", ft_strrchr_high),
	TEST("every char, never outside the string [guarded]", "Scanning backwards must stop at s[0]: with the \
string right after a protected page, reading s[-1] is a segfault.", ft_strrchr_guard),
	TEST("8 MB string", "First / last / absent char of a big string (a match at index 0 is the classic \
backwards-loop bug).", ft_strrchr_big),
};

/* ================================================================== */
/* ft_strncmp / ft_memcmp / ft_memchr                                 */
/* ================================================================== */

static void	ncmp_one(const char *a, const char *b, size_t n, int front)
{
	int	exp;
	int	got;

	CASE("ft_strncmp(\"%s\", \"%s\", %zu) [%s]", t_esc(a), t_esc(b), n, front ? "front guarded" : "guarded");
	exp = sign(strncmp(a, b, n));
	if (front)
		got = sign(ft_strncmp(t_gstr_front(a), t_gstr_front(b), n));
	else
		got = sign(ft_strncmp(t_gstr(a), t_gstr(b), n));
	EXPECT(got == exp, "expected a result %s, got %s", sgn(exp), sgn(got));
}

static void	ncmp_equal(void)
{
	ncmp_one("abc", "abc", 3, 0);
	ncmp_one("abc", "abc", 10, 1);
	ncmp_one("", "", 5, 0);
	ncmp_one("Hello World", "Hello World", 11, 0);
	ncmp_one("\xff\x80", "\xff\x80", 2, 1);
}

static void	ncmp_diff(void)
{
	ncmp_one("abc", "abd", 3, 0);
	ncmp_one("abd", "abc", 3, 1);
	ncmp_one("1234", "1235", 4, 0);
	ncmp_one("abcdef", "abcxyz", 6, 0);
	ncmp_one("a", "b", 1, 1);
	ncmp_one("Z", "a", 1, 0);
}

static void	ncmp_unsigned(void)
{
	ncmp_one("\200", "\0", 1, 0);
	ncmp_one("\0", "\200", 1, 1);
	ncmp_one("\xff", "\x01", 1, 0);
	ncmp_one("test\200", "test\0", 6, 0);
	ncmp_one("a\x7f", "a\x80", 2, 1);
}

static void	ncmp_zero(void)
{
	int	got;

	ncmp_one("abc", "xyz", 0, 0);
	ncmp_one("", "a", 0, 1);
	CASE("ft_strncmp(<protected page>, <protected page>, 0)");
	got = ft_strncmp(t_gmem(NULL, 0), t_gmem(NULL, 0), 0);
	EXPECT(got == 0, "expected 0, nothing may be read, got %d", got);
}

static void	ncmp_n(void)
{
	ncmp_one("abcX", "abcY", 3, 0);
	ncmp_one("abcX", "abcY", 4, 0);
	ncmp_one("12345", "12399", 3, 1);
	ncmp_one("12345", "12399", 4, 0);
	ncmp_one("x", "y", 1, 0);
}

static void	ncmp_stop(void)
{
	ncmp_one("abc", "abc", 100, 0);
	ncmp_one("abc", "abc", 100, 1);
	ncmp_one("ab", "abc", 10, 0);
	ncmp_one("abc", "ab", 10, 1);
	ncmp_one("", "", 100, 0);
	ncmp_one("", "abc", 3, 1);
}

static void	ncmp_max(void)
{
	ncmp_one("abc", "abc", SIZE_MAX, 0);
	ncmp_one("abc", "abd", SIZE_MAX, 1);
	ncmp_one("", "", SIZE_MAX, 0);
	ncmp_one("abc", "ab", SIZE_MAX, 0);
}

static void	ncmp_table(void)
{
	const char	*p[][2] = {{"abc", "abc"}, {"abc", "abd"}, {"abd", "abc"}, {"abc", "ab"},
		{"ab", "abc"}, {"", ""}, {"", "a"}, {"a", ""}, {"\200", "\0"}, {"\0", "\200"},
		{"\xff", "\x01"}, {"test\200", "test\0"}, {"abcdef", "abcxyz"}, {"1234", "1235"}};
	size_t		ns[] = {0, 1, 2, 3, 4, 5, 6, 10, 42, SIZE_MAX};
	size_t		i;
	size_t		k;

	i = 0;
	while (i < sizeof(p) / sizeof(*p))
	{
		k = 0;
		while (k < sizeof(ns) / sizeof(*ns))
		{
			ncmp_one(p[i][0], p[i][1], ns[k], 0);
			ncmp_one(p[i][0], p[i][1], ns[k], 1);
			k++;
		}
		i++;
	}
}

static const t_test	g_ft_strncmp[] = {
	TEST("equal strings give 0", "Identical strings (and n past their end) compare equal.", ncmp_equal),
	TEST("sign of the first difference", "The sign of the result is the sign of the first differing \
bytes (only the sign is checked, not the exact value).", ncmp_diff),
	TEST("bytes compared as unsigned char", "\\200 is greater than \\0 and \\xff greater than \\x01: \
subtracting (signed) chars gives the wrong sign.", ncmp_unsigned),
	TEST("n = 0 gives 0", "With n = 0 nothing is compared (nor read) and the result is 0.", ncmp_zero),
	TEST("only the first n chars count", "\"abcX\" and \"abcY\" are equal for n = 3, different for n = 4.", ncmp_n),
	TEST("stops at the '\\0' [guarded]", "With n bigger than the strings, it must stop at the first \
'\\0': the strings end at a protected page.", ncmp_stop),
	TEST("n = SIZE_MAX", "A huge n must not overflow a counter (int i < n) or wrap.", ncmp_max),
	TEST("full table vs libc [guarded, front guarded]", "14 string pairs x 10 values of n, compared with \
the libc.", ncmp_table),
};

static void	mcmp_one(const char *a, const char *b, size_t n)
{
	int	exp;
	int	got;

	CASE("ft_memcmp(\"%s\", \"%s\", %zu) [guarded]", t_escn(a, n), t_escn(b, n), n);
	exp = sign(memcmp(a, b, n));
	got = sign(ft_memcmp(t_gmem(a, n), t_gmem(b, n), n));
	EXPECT(got == exp, "expected a result %s, got %s", sgn(exp), sgn(got));
}

static void	mcmp_equal(void)
{
	mcmp_one("abc", "abc", 3);
	mcmp_one("a\0b", "a\0b", 3);
	mcmp_one("\xff\xff", "\xff\xff", 2);
	mcmp_one("x", "x", 1);
}

static void	mcmp_diff(void)
{
	mcmp_one("abc", "abd", 3);
	mcmp_one("abd", "abc", 3);
	mcmp_one("zbc", "abc", 3);
	mcmp_one("12345678", "12345679", 8);
}

static void	mcmp_unsigned(void)
{
	mcmp_one("\x80", "\x01", 1);
	mcmp_one("\x01", "\x80", 1);
	mcmp_one("\xff\xff", "\xff\xfe", 2);
	mcmp_one("t\200", "t\0", 2);
}

static void	mcmp_nul(void)
{
	mcmp_one("a\0b", "a\0c", 3);
	mcmp_one("a\0c", "a\0b", 3);
	mcmp_one("\0\0\0x", "\0\0\0y", 4);
}

static void	mcmp_zero(void)
{
	int	got;

	CASE("ft_memcmp(<protected page>, <protected page>, 0)");
	got = ft_memcmp(t_gmem(NULL, 0), t_gmem(NULL, 0), 0);
	EXPECT(got == 0, "expected 0, got %d", got);
	mcmp_one("abc", "xyz", 0);
}

static void	mcmp_all(void)
{
	unsigned char	a[64];
	unsigned char	b[64];
	size_t			n;
	size_t			d;
	int				exp;
	int				got;

	n = 1;
	while (n <= 64)
	{
		d = 0;
		while (d < n)
		{
			fill_pattern(a, n, 4);
			fill_pattern(b, n, 4);
			b[d] ^= 0x80;
			CASE("ft_memcmp(<%zu bytes>, <same, byte %zu differs>, %zu) [guarded]", n, d, n);
			exp = sign(memcmp(a, b, n));
			got = sign(ft_memcmp(t_gmem(a, n), t_gmem(b, n), n));
			EXPECT(got == exp, "expected a result %s, got %s", sgn(exp), sgn(got));
			d++;
		}
		n++;
	}
}

static void	mcmp_big(void)
{
	unsigned char	*a;
	unsigned char	*b;
	size_t			n;

	n = 16 << 20;
	a = t_gmem(NULL, n);
	b = t_gmem(NULL, n);
	memset(a, 'm', n);
	memset(b, 'm', n);
	CASE("ft_memcmp(<16 MB>, <same>, 16 MB) [guarded]");
	EXPECT(ft_memcmp(a, b, n) == 0, "expected 0");
	b[n - 1] = 'n';
	CASE("ft_memcmp(<16 MB>, <last byte differs>, 16 MB) [guarded]");
	EXPECT(ft_memcmp(a, b, n) < 0, "expected < 0");
}

static const t_test	g_ft_memcmp[] = {
	TEST("equal buffers give 0", "Identical bytes compare equal, even with '\\0' inside.", mcmp_equal),
	TEST("sign of the first difference", "The sign of the result is the sign of the first differing byte.", mcmp_diff),
	TEST("bytes compared as unsigned char", "0x80 is greater than 0x01: comparing signed chars gives the \
wrong sign.", mcmp_unsigned),
	TEST("'\\0' is not special", "memcmp compares n bytes, it does not stop at '\\0' like strncmp.", mcmp_nul),
	TEST("n = 0 gives 0", "With n = 0 nothing is read (the pointers are at a protected page) and 0 is \
returned.", mcmp_zero),
	TEST("difference at every position, never past n [guarded]", "Every length 1..64 with the difference \
at every position: buffers end at a protected page, reading byte n is a segfault.", mcmp_all),
	TEST_SLOW("16 MB", "Equal big buffers, then a difference on the very last byte.", mcmp_big),
};

static void	mchr_one(const void *buf, size_t size, int c, size_t n)
{
	char		*g;
	const void	*exp;
	void		*got;

	g = t_gmem(buf, size);
	CASE("ft_memchr(\"%s\", %d, %zu) [guarded]", t_escn(buf, size), c, n);
	exp = memchr(buf, c, n);
	got = ft_memchr(g, c, n);
	if (!exp)
		EXPECT(got == NULL, "expected NULL, got index %td", (char *)got - g);
	else
		EXPECT(got == g + ((const char *)exp - (const char *)buf), "expected index %td, got %s",
			(const char *)exp - (const char *)buf, got ? "another pointer" : "NULL");
}

static void	mchr_found(void)
{
	mchr_one("abcabc", 6, 'b', 6);
	mchr_one("abcabc", 6, 'a', 6);
	mchr_one("abcabc", 6, 'c', 6);
	mchr_one("x", 1, 'x', 1);
}

static void	mchr_conv(void)
{
	mchr_one("abc\xc8" "def", 7, 0xc8, 7);
	mchr_one("abc\xc8" "def", 7, -56, 7);
	mchr_one("abcdef", 6, 'd' + 256, 6);
	mchr_one("\xff\x01", 2, -1, 2);
	mchr_one("abc", 3, 'a' - 256, 3);
}

static void	mchr_nul(void)
{
	mchr_one("abc\0def", 7, 'e', 7);
	mchr_one("abc\0def", 7, 0, 7);
	mchr_one("\0\0x", 3, 'x', 3);
}

static void	mchr_absent(void)
{
	size_t	n;

	n = 0;
	while (n <= 7)
	{
		mchr_one("abcdefg", 7, 'z', n);
		mchr_one("abcdefg", 7, 'g', n);
		n++;
	}
}

static void	mchr_zero(void)
{
	void	*got;

	CASE("ft_memchr(<protected page>, 'a', 0)");
	got = ft_memchr(t_gmem(NULL, 0), 'a', 0);
	EXPECT(got == NULL, "expected NULL");
	mchr_one("abc", 3, 'a', 0);
}

static void	mchr_table(void)
{
	const char	buf[] = "abc\0def\xc8";
	int			cs[] = {'a', 'c', 0, 'd', 0xc8, -56, 'd' + 256, 'z', 'f'};
	size_t		n;
	size_t		k;

	k = 0;
	while (k < sizeof(cs) / sizeof(*cs))
	{
		n = 0;
		while (n <= 8)
			mchr_one(buf, 8, cs[k], n++);
		k++;
	}
}

static void	mchr_big(void)
{
	unsigned char	*g;
	size_t			n;

	n = 16 << 20;
	g = t_gmem(NULL, n);
	memset(g, 'm', n);
	g[n - 1] = 'Q';
	CASE("ft_memchr(<16 MB>, 'Q', 16 MB) (last byte) [guarded]");
	EXPECT(ft_memchr(g, 'Q', n) == g + n - 1, "wrong pointer");
	CASE("ft_memchr(<16 MB>, 'Z', 16 MB) [guarded]");
	EXPECT(ft_memchr(g, 'Z', n) == NULL, "expected NULL");
}

static void	mchr_c11(void)
{
	char	*g;

	g = t_gmem("abcx", 4);
	CASE("ft_memchr(<4 bytes ending in 'x'>, 'x', SIZE_MAX) [guarded]");
	EXPECT(ft_memchr(g, 'x', SIZE_MAX) == g + 3, "should stop at the first match (C11 7.24.5.1)");
}

static const t_test	g_ft_memchr[] = {
	TEST("finds the first occurrence", "memchr returns a pointer to the first byte equal to c.", mchr_found),
	TEST("c converted to unsigned char", "c = 0xc8, -56 and 0xc8 + 256 all look for the byte 0xc8.", mchr_conv),
	TEST("'\\0' is not special", "memchr searches n bytes: it goes past '\\0' and can find '\\0' itself.", mchr_nul),
	TEST("not found within n: NULL, never past n", "A byte after the first n bytes is not found, and \
reading it is a segfault (protected page).", mchr_absent),
	TEST("n = 0 gives NULL", "With n = 0 nothing is read and NULL is returned.", mchr_zero),
	TEST("full table vs libc [guarded]", "9 values of c x every n from 0 to 8 on \"abc\\0def\\xc8\".", mchr_table),
	TEST_SLOW("16 MB", "A match on the very last byte, and no match at all.", mchr_big),
	TEST_UB("n = SIZE_MAX stops at the first match", "C11 says memchr stops at the first match, so a \
too big n is fine when c is there. Implementations reading ahead crash.", mchr_c11),
};

/* ================================================================== */
/* ft_strnstr                                                         */
/* ================================================================== */

static void	nstr_one(const char *big, const char *little, size_t len, int front)
{
	char		*gb;
	const char	*exp;
	char		*got;

	gb = front ? t_gstr_front(big) : t_gstr(big);
	CASE("ft_strnstr(\"%s\", \"%s\", %zu) [%s]", t_esc(big), t_esc(little), len,
		front ? "front guarded" : "guarded");
	exp = ref_strnstr(big, little, len);
	got = ft_strnstr(gb, front ? t_gstr_front(little) : t_gstr(little), len);
	if (!exp)
		EXPECT(got == NULL, "expected NULL, got \"%s\"", t_esc(got));
	else
		EXPECT(got == gb + (exp - big), "expected a pointer to \"%s\", got %s\"%s\"",
			t_esc(exp), got ? "" : "NULL ", t_esc(got));
}

static void	nstr_found(void)
{
	nstr_one("Hello World", "World", 11, 0);
	nstr_one("Hello World", "Hello", 11, 1);
	nstr_one("Hello World", "o", 11, 0);
	nstr_one("lorem ipsum dolor sit amet", "dolor", 30, 0);
	nstr_one("abc", "abc", 3, 1);
}

static void	nstr_empty(void)
{
	nstr_one("Hello", "", 5, 0);
	nstr_one("Hello", "", 0, 1);
	nstr_one("", "", 0, 0);
	nstr_one("", "", 10, 1);
}

static void	nstr_absent(void)
{
	nstr_one("Hello World", "world", 11, 0);
	nstr_one("Hello World", "Worlds", 20, 1);
	nstr_one("", "a", 5, 0);
	nstr_one("aaaa", "b", 4, 0);
}

static void	nstr_len(void)
{
	nstr_one("Hello World", "World", 10, 0);
	nstr_one("Hello World", "World", 7, 1);
	nstr_one("Hello World", "World", 6, 0);
	nstr_one("abcdef", "cd", 3, 0);
	nstr_one("abcdef", "cd", 4, 1);
	nstr_one("aaabcabcd", "abcd", 8, 0);
	nstr_one("aaabcabcd", "abcd", 9, 0);
}

static void	nstr_zero(void)
{
	nstr_one("Hello", "H", 0, 0);
	nstr_one("Hello", "Hello", 0, 1);
	nstr_one("", "x", 0, 0);
}

static void	nstr_partial(void)
{
	nstr_one("aaab", "aab", 4, 0);
	nstr_one("aaaaab", "aab", 6, 1);
	nstr_one("MZIRIBMZIRIBMZE123", "MZIRIBMZE", 18, 0);
	nstr_one("ababac", "abac", 6, 0);
	nstr_one("abcabcabd", "abcabd", 9, 1);
	nstr_one("mississippi", "issip", 11, 0);
}

static void	nstr_bigger(void)
{
	nstr_one("abc", "abcd", 10, 0);
	nstr_one("abc", "abcd", SIZE_MAX, 1);
	nstr_one("a", "aa", 2, 0);
	nstr_one("", "abc", 3, 0);
}

static void	nstr_max(void)
{
	nstr_one("Hello World", "World", SIZE_MAX, 0);
	nstr_one("Hello World", "xyz", SIZE_MAX, 1);
	nstr_one("aaab", "aab", SIZE_MAX, 0);
	nstr_one("ab", "b", 1000, 1);
}

static void	nstr_table(void)
{
	const char	*bigs[] = {"", "a", "aaab", "aaaaab", "Hello World", "lorem ipsum dolor sit amet",
		"MZIRIBMZIRIBMZE123"};
	const char	*lits[] = {"", "a", "b", "ab", "aab", "World", "dolor", "lorem ipsum dolor sit amet!",
		"o", "MZIRIBMZE", "d", "lorem"};
	size_t		lens[] = {0, 1, 2, 3, 4, 5, 6, 7, 10, 11, 12, 15, 17, 30, SIZE_MAX};
	size_t		i;
	size_t		j;
	size_t		k;

	i = 0;
	while (i < sizeof(bigs) / sizeof(*bigs))
	{
		j = 0;
		while (j < sizeof(lits) / sizeof(*lits))
		{
			k = 0;
			while (k < sizeof(lens) / sizeof(*lens))
			{
				nstr_one(bigs[i], lits[j], lens[k], (int)(k % 2));
				k++;
			}
			j++;
		}
		i++;
	}
}

static void	nstr_null(void)
{
	CASE("ft_strnstr(NULL, \"abc\", 0)");
	EXPECT(ft_strnstr(NULL, "abc", 0) == NULL, "should return NULL (like BSD)");
}

static const t_test	g_ft_strnstr[] = {
	TEST("finds little in big", "Returns a pointer to the first occurrence of little in big.", nstr_found),
	TEST("empty little returns big", "If little is \"\", big itself is returned (even with len 0).", nstr_empty),
	TEST("not found returns NULL", "Case sensitive, and NULL when little is not there.", nstr_absent),
	TEST("little must fit entirely in len", "A match that ends after big[len - 1] does not count: \
\"World\" is not in the first 10 chars of \"Hello World\".", nstr_len),
	TEST("len = 0", "With len 0 nothing can be found (unless little is empty).", nstr_zero),
	TEST("partial matches (backtracking)", "\"aab\" in \"aaab\": after a partial match the search must \
restart at the next char, not after the partial match.", nstr_partial),
	TEST("little longer than big", "It can't be found, and nothing past big's '\\0' may be read.", nstr_bigger),
	TEST("len bigger than big [guarded]", "len = SIZE_MAX: the search must stop at big's '\\0' (a \
protected page follows).", nstr_max),
	TEST("full table vs BSD [guarded, front guarded]", "7 bigs x 12 littles x 15 lens compared with a \
BSD reference.", nstr_table),
	TEST_UB("NULL big with len 0", "BSD returns NULL without reading big when len is 0: evaluators try it.", nstr_null),
};

/* ================================================================== */
/* ft_atoi                                                            */
/* ================================================================== */

static void	atoi_list(const char *const *strs, size_t n)
{
	size_t	i;
	int		exp;
	int		got;

	i = 0;
	while (i < n)
	{
		exp = atoi(strs[i]);
		CASE("ft_atoi(\"%s\") [guarded]", t_esc(strs[i]));
		got = ft_atoi(t_gstr(strs[i]));
		EXPECT(got == exp, "expected %d, got %d", exp, got);
		CASE("ft_atoi(\"%s\") [front guarded]", t_esc(strs[i]));
		got = ft_atoi(t_gstr_front(strs[i]));
		EXPECT(got == exp, "expected %d, got %d", exp, got);
		i++;
	}
}

#define ATOI(NAME, ...) \
static void	NAME(void) \
{ \
	static const char	*s[] = {__VA_ARGS__}; \
\
	atoi_list(s, sizeof(s) / sizeof(*s)); \
}

ATOI(atoi_simple, "0", "1", "42", "-42", "9", "-9", "10", "-10", "12345", "-12345", "1000000000")
ATOI(atoi_space, " 42", "\t42", "\n42", "\v42", "\f42", "\r42", "\t\n\v\f\r 42", "   -42",
	" \t\v\f\r\n  +0012a", "\n\n\n  -46\b9 \n5d6")
ATOI(atoi_sign, "+42", "+-42", "-+42", "--42", "++42", "+ 42", "- 42", " - 42", "-", "+", "-0", "+0")
ATOI(atoi_stop, "42abc", "4 2", "12 34", "1\n2", "-3.14", "0x1A", "99bottles", "4\x80" "2", "7-3")
ATOI(atoi_limits, "2147483647", "-2147483648", "-2147483647", "2147483646", "+2147483647", "  -2147483648x")
ATOI(atoi_zeros, "00000000000000000000000000042", "-0000000000000000000000000042",
	"+000000000000000000000000000000000000000002147483647", "0000", "-00")
ATOI(atoi_nothing, "", " ", "\t\n", "abc42", "a1", "- ", "+\t1", "\x1b 42")
ATOI(atoi_notspace, "\x80 42", "\xe2\x80\x83 1", "\x7f" "42", "\xa0" "42", "\x85" "7", "\x0e" "3", "\x1c" "5")

static void	atoi_overflow(void)
{
	const char	*strs[] = {"2147483648", "-2147483649", "9999999999", "99999999999999999999",
		"-99999999999999999999", "18446744073709551616", "9223372036854775808",
		"123456789012345678901234567890"};
	size_t		i;

	i = 0;
	while (i < sizeof(strs) / sizeof(*strs))
	{
		CASE("ft_atoi(\"%s\") (overflow is UB, it only must not crash)", strs[i]);
		(void)ft_atoi(t_gstr(strs[i]));
		i++;
	}
}

static const t_test	g_ft_atoi[] = {
	TEST("simple numbers", "Positive and negative numbers, compared with the libc atoi.", atoi_simple),
	TEST("leading whitespace", "The 6 isspace() chars ' ' \\t \\n \\v \\f \\r are skipped before the number, \
nothing else.", atoi_space),
	TEST("at most one sign", "One '+' or '-' is allowed right before the digits: \"+-42\", \"--42\", \
\"+ 42\" give 0.", atoi_sign),
	TEST("stops at the first non-digit", "Digits are read until the first other char: \"42abc\" is 42, \
\"4 2\" is 4.", atoi_stop),
	TEST("INT_MAX and INT_MIN", "\"-2147483648\" must give INT_MIN: accumulating a positive int and \
negating at the end overflows.", atoi_limits),
	TEST("leading zeros", "Any number of leading zeros, also before INT_MAX.", atoi_zeros),
	TEST("no number gives 0", "\"\", only spaces, only a sign, letters first: 0.", atoi_nothing),
	TEST("non-ASCII / control chars are not spaces", "Bytes like 0x80, 0xa0 (nbsp) or 0x1b (ESC) before \
the digits stop the parsing: 0. A signed char passed to a space table breaks here.", atoi_notspace),
	TEST_UB("overflow does not crash", "Beyond INT_MAX the result is undefined, but it must not crash \
(evaluators try it).", atoi_overflow),
};

/* ================================================================== */
/* ft_calloc / ft_strdup                                              */
/* ================================================================== */

static void	check_calloc(size_t nmemb, size_t size)
{
	unsigned char	*p;

	CASE("ft_calloc(%zu, %zu)", nmemb, size);
	p = ft_calloc(nmemb, size);
	if (!p)
	{
		t_fail("returned NULL");
		return ;
	}
	if (!t_is_block(p))
	{
		t_fail("result was not allocated with malloc() (calloc/realloc are forbidden)");
		return ;
	}
	EXPECT(t_block_size(p) >= nmemb * size, "allocated %zu byte(s), needs %zu", t_block_size(p), nmemb * size);
	EXPECT(all_bytes(p, nmemb * size, 0), "memory is not zeroed (the tester's malloc returns garbage on purpose)");
	free(p);
}

static void	calloc_zeroed(void)
{
	check_calloc(5, 4);
	check_calloc(1, 1);
	check_calloc(42, 1);
	check_calloc(1, 42);
	check_calloc(3, sizeof(long));
	check_calloc(7, 3);
	check_calloc(1000, 1000);
}

static void	calloc_size(void)
{
	size_t	args[][2] = {{1, 1}, {10, 4}, {4, 10}, {3, 7}, {100, 3}, {1, 4096}};
	size_t	i;
	void	*p;

	i = 0;
	while (i < sizeof(args) / sizeof(*args))
	{
		CASE("ft_calloc(%zu, %zu)", args[i][0], args[i][1]);
		p = ft_calloc(args[i][0], args[i][1]);
		EXPECT(p && t_is_block(p), "must return a pointer from malloc()");
		EXPECT(!p || t_block_size(p) >= args[i][0] * args[i][1],
			"allocated %zu byte(s) for nmemb * size = %zu", t_block_size(p), args[i][0] * args[i][1]);
		t_free(p);
		i++;
	}
}

static void	calloc_zero(void)
{
	void	*p[3];
	size_t	args[3][2] = {{0, 0}, {0, 5}, {5, 0}};
	int		i;

	i = 0;
	while (i < 3)
	{
		CASE("ft_calloc(%zu, %zu)", args[i][0], args[i][1]);
		p[i] = ft_calloc(args[i][0], args[i][1]);
		EXPECT(p[i] != NULL, "must return a unique pointer that can be passed to free(), got NULL");
		EXPECT(!p[i] || t_is_block(p[i]), "the returned pointer can't be passed to free()");
		i++;
	}
	CASE("ft_calloc(0, 0), (0, 5), (5, 0)");
	EXPECT(!p[0] || !p[1] || !p[2] || (p[0] != p[1] && p[1] != p[2] && p[0] != p[2]),
		"the pointers are not unique");
	t_free(p[0]);
	t_free(p[1]);
	t_free(p[2]);
}

static void	calloc_overflow(void)
{
	size_t	args[][2] = {{SIZE_MAX / 2 + 1, 2}, {SIZE_MAX, SIZE_MAX}, {SIZE_MAX, 2},
		{2, SIZE_MAX}, {(size_t)1 << 32, (size_t)1 << 32}, {(size_t)1 << 33, (size_t)1 << 31},
		{(size_t)-10, sizeof(int)}, {SIZE_MAX / 3 + 1, 3}};
	size_t	i;
	void	*p;

	i = 0;
	while (i < sizeof(args) / sizeof(*args))
	{
		CASE("ft_calloc(%zu, %zu)", args[i][0], args[i][1]);
		p = ft_calloc(args[i][0], args[i][1]);
		EXPECT(p == NULL, "nmemb * size overflows: expected NULL, got a %zu-byte block", t_block_size(p));
		t_free(p);
		i++;
	}
}

static void	calloc_huge(void)
{
	size_t	args[][2] = {{(size_t)1 << 40, 1}, {1, (size_t)1 << 40}, {SIZE_MAX, 1}, {1, SIZE_MAX},
		{(size_t)1 << 20, (size_t)1 << 20}};
	size_t	i;
	void	*p;

	i = 0;
	while (i < sizeof(args) / sizeof(*args))
	{
		CASE("ft_calloc(%zu, %zu) (malloc returns NULL)", args[i][0], args[i][1]);
		p = ft_calloc(args[i][0], args[i][1]);
		EXPECT(p == NULL, "expected NULL");
		t_free(p);
		i++;
	}
}

static void	calloc_big(void)
{
	check_calloc(4 << 20, 8);
}

static void	mf_calloc(void)
{
	void	*r;

	CASE("ft_calloc(10, sizeof(int)), its malloc fails");
	t_arm();
	r = ft_calloc(10, sizeof(int));
	t_disarm();
	if (t_injected())
		EXPECT(r == NULL, "returned non-NULL although its malloc returned NULL");
	else
		EXPECT(r && t_is_block(r), "expected a malloc'd block");
	t_free(r);
}

static void	mf_calloc0(void)
{
	void	*r;

	CASE("ft_calloc(0, 0), its malloc fails");
	t_arm();
	r = ft_calloc(0, 0);
	t_disarm();
	if (t_injected())
		EXPECT(r == NULL, "returned non-NULL although its malloc returned NULL");
	t_free(r);
}

static const t_test	g_ft_calloc[] = {
	TEST("memory is zeroed", "The tester's malloc fills every block with garbage (0xBE): calloc must \
zero all nmemb * size bytes.", calloc_zeroed),
	TEST("comes from malloc, big enough", "The block must come from malloc (calloc / realloc are \
forbidden) and hold at least nmemb * size bytes.", calloc_size),
	TEST("calloc(0, x) / (x, 0): unique pointer", "The subject (and the libc) return a unique pointer \
that can be passed to free(), not NULL.", calloc_zero),
	TEST("nmemb * size overflow gives NULL", "SIZE_MAX / 2 + 1 elements of 2 bytes overflow size_t to 0: \
without an overflow check calloc returns a tiny block and the caller writes way past it.", calloc_overflow),
	TEST("malloc returning NULL: no crash", "When the size is too big, malloc returns NULL: calloc must \
return NULL, not bzero(NULL, n).", calloc_huge),
	TEST_SLOW("32 MB zeroed", "A big block, zeroed completely.", calloc_big),
	TEST_MF("malloc fails: calloc(10, 4)", "When malloc fails, calloc returns NULL (and does not touch \
the NULL pointer).", mf_calloc),
	TEST_MF("malloc fails: calloc(0, 0)", "Same for the unique pointer of calloc(0, 0).", mf_calloc0),
};

static void	strdup_copy(void)
{
	const char	*strs[] = {"a", "hello", "Hello World 42!", "lorem ipsum dolor sit amet"};
	size_t		i;

	i = 0;
	while (i < sizeof(strs) / sizeof(*strs))
	{
		CASE("ft_strdup(\"%s\") [guarded]", t_esc(strs[i]));
		t_check_str("", ft_strdup(t_gstr(strs[i])), strs[i]);
		i++;
	}
}

static void	strdup_new(void)
{
	char	*g;
	char	*r;

	g = t_gstr("hello");
	CASE("ft_strdup(\"hello\"), then modify the copy");
	r = ft_strdup(g);
	if (!r)
	{
		t_fail("returned NULL");
		return ;
	}
	EXPECT(r != g, "must return a new copy, not its argument");
	r[0] = 'J';
	EXPECT(!strcmp(g, "hello"), "changing the copy changed the original");
	t_free(r);
}

static void	strdup_empty(void)
{
	CASE("ft_strdup(\"\") [guarded]");
	t_check_str("", ft_strdup(t_gstr("")), "");
	CASE("ft_strdup(\"\") [front guarded]");
	t_check_str("", ft_strdup(t_gstr_front("")), "");
}

static void	strdup_high(void)
{
	CASE("ft_strdup(\"\\xff\\x80\\x01\")");
	t_check_str("", ft_strdup(t_gstr("\xff\x80\x01")), "\xff\x80\x01");
	CASE("ft_strdup(\"caf\\xc3\\xa9\")");
	t_check_str("", ft_strdup(t_gstr_front("caf\xc3\xa9")), "caf\xc3\xa9");
}

static void	strdup_guard(void)
{
	char	buf[80];
	size_t	n;

	n = 0;
	while (n < 70)
	{
		memset(buf, 'a' + n % 26, n);
		buf[n] = 0;
		CASE("ft_strdup(<%zu chars>) [guarded]", n);
		t_check_str("", ft_strdup(t_gstr(buf)), buf);
		CASE("ft_strdup(<%zu chars>) [front guarded]", n);
		t_check_str("", ft_strdup(t_gstr_front(buf)), buf);
		n++;
	}
}

static void	strdup_big(void)
{
	char	*g;
	char	*r;
	size_t	n;

	n = 8 << 20;
	g = big_gstr(n, 'k');
	CASE("ft_strdup(<8 MB string>) [guarded]");
	r = ft_strdup(g);
	EXPECT(r && t_block_size(r) >= n + 1 && memcmp(r, g, n + 1) == 0, "wrong copy");
	t_free(r);
}

static void	mf_strdup(void)
{
	CASE("ft_strdup(\"hello\"), its malloc fails");
	MF_STR(ft_strdup(t_gstr("hello")), "hello");
}

static const t_test	g_ft_strdup[] = {
	TEST("copies the string", "The copy holds the same chars and its '\\0', in a block big enough \
(strlen + 1).", strdup_copy),
	TEST("a new, independent copy", "The result is a new malloc'd string: changing it does not change \
the original.", strdup_new),
	TEST("empty string", "strdup(\"\") returns a malloc'd \"\" (1 byte), not a literal and not NULL.", strdup_empty),
	TEST("bytes > 127", "Every byte is copied as is.", strdup_high),
	TEST("lengths 0..69 [guarded, front guarded]", "The source is read up to its '\\0' and not one byte \
more (protected page).", strdup_guard),
	TEST_SLOW("8 MB string", "A big string copied completely.", strdup_big),
	TEST_MF("malloc fails", "When its malloc fails, strdup returns NULL.", mf_strdup),
};

/* ================================================================== */

void	run_part1(void)
{
	t_section("PART 1 - LIBC FUNCTIONS");
	GROUP("ft_isalpha", g_ft_isalpha);
	GROUP("ft_isdigit", g_ft_isdigit);
	GROUP("ft_isalnum", g_ft_isalnum);
	GROUP("ft_isascii", g_ft_isascii);
	GROUP("ft_isprint", g_ft_isprint);
	GROUP("ft_strlen", g_ft_strlen);
	GROUP("ft_memset", g_ft_memset);
	GROUP("ft_bzero", g_ft_bzero);
	GROUP("ft_memcpy", g_ft_memcpy);
	GROUP("ft_memmove", g_ft_memmove);
	GROUP("ft_strlcpy", g_ft_strlcpy);
	GROUP("ft_strlcat", g_ft_strlcat);
	GROUP("ft_toupper", g_ft_toupper);
	GROUP("ft_tolower", g_ft_tolower);
	GROUP("ft_strchr", g_ft_strchr);
	GROUP("ft_strrchr", g_ft_strrchr);
	GROUP("ft_strncmp", g_ft_strncmp);
	GROUP("ft_memchr", g_ft_memchr);
	GROUP("ft_memcmp", g_ft_memcmp);
	GROUP("ft_strnstr", g_ft_strnstr);
	GROUP("ft_atoi", g_ft_atoi);
	GROUP("ft_calloc", g_ft_calloc);
	GROUP("ft_strdup", g_ft_strdup);
}
