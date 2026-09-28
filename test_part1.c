#include "tester.h"

/*
** PART 1 - Libc functions
** Every result is compared with the real libc (or with a reference
** implementation for the BSD-only strlcpy / strlcat / strnstr).
** "[guarded]" means the input ends (or starts) exactly at a PROT_NONE
** page: reading or writing a single byte too far is a SEGFAULT.
*/

static int	sign(int x)
{
	return ((x > 0) - (x < 0));
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

/* ------------------------------------------------------------------ */
/* ft_is* / ft_to*                                                    */
/* ------------------------------------------------------------------ */

static int	ref_isascii(int c)
{
	return (c >= 0 && c <= 127);
}

static void	check_class(const char *name, int (*ft)(int), int (*ref)(int))
{
	int	c;
	int	got;
	int	exp;

	c = -1;
	while (c <= 255)
	{
		CASE("%s(%d)", name, c);
		got = ft(c);
		exp = ref(c) ? 1 : 0;
		EXPECT(got == exp, "expected %d, got %d (the subject wants exactly 1 or 0)", exp, got);
		c++;
	}
}

static void	check_class_outside(const char *name, int (*ft)(int))
{
	int	c;

	c = -300;
	while (c <= 600)
	{
		if (c < -1 || c > 255)
		{
			CASE("%s(%d)", name, c);
			EXPECT(ft(c) == 0, "values outside unsigned char should give 0, got %d", ft(c));
		}
		c++;
	}
}

static int	w_isalpha(int c) { return (isalpha(c)); }
static int	w_isdigit(int c) { return (isdigit(c)); }
static int	w_isalnum(int c) { return (isalnum(c)); }
static int	w_isprint(int c) { return (isprint(c)); }

static void	test_isalpha(void) { check_class("ft_isalpha", ft_isalpha, w_isalpha); }
static void	test_isdigit(void) { check_class("ft_isdigit", ft_isdigit, w_isdigit); }
static void	test_isalnum(void) { check_class("ft_isalnum", ft_isalnum, w_isalnum); }
static void	test_isascii(void) { check_class("ft_isascii", ft_isascii, ref_isascii); }
static void	test_isprint(void) { check_class("ft_isprint", ft_isprint, w_isprint); }

static void	test_is_outside(void)
{
	check_class_outside("ft_isalpha", ft_isalpha);
	check_class_outside("ft_isdigit", ft_isdigit);
	check_class_outside("ft_isalnum", ft_isalnum);
	check_class_outside("ft_isascii", ft_isascii);
	check_class_outside("ft_isprint", ft_isprint);
}

static void	test_toupper_tolower(void)
{
	int	c;

	c = -1;
	while (c <= 255)
	{
		CASE("ft_toupper(%d)", c);
		EXPECT(ft_toupper(c) == toupper(c), "expected %d, got %d", toupper(c), ft_toupper(c));
		CASE("ft_tolower(%d)", c);
		EXPECT(ft_tolower(c) == tolower(c), "expected %d, got %d", tolower(c), ft_tolower(c));
		c++;
	}
}

static void	test_toupper_tolower_outside(void)
{
	int	c;

	c = -300;
	while (c <= 600)
	{
		if (c < -1 || c > 255)
		{
			CASE("ft_toupper(%d)", c);
			EXPECT(ft_toupper(c) == c, "should return c unchanged, got %d", ft_toupper(c));
			CASE("ft_tolower(%d)", c);
			EXPECT(ft_tolower(c) == c, "should return c unchanged, got %d", ft_tolower(c));
		}
		c++;
	}
}

/* ------------------------------------------------------------------ */
/* ft_strlen                                                          */
/* ------------------------------------------------------------------ */

static void	test_strlen(void)
{
	char	buf[400];
	char	*g;
	size_t	n;

	n = 0;
	while (n < 300)
	{
		memset(buf, 'a' + n % 26, n);
		buf[n] = 0;
		CASE("ft_strlen(<%zu chars>) [guarded]", n);
		g = t_gstr(buf);
		EXPECT(ft_strlen(g) == n, "expected %zu, got %zu", n, ft_strlen(g));
		n++;
	}
	CASE("ft_strlen(\"\\xff\\x80\\x01\") [guarded]");
	EXPECT(ft_strlen(t_gstr("\xff\x80\x01")) == 3, "expected 3");
	CASE("ft_strlen(\"\") [front guarded]");
	EXPECT(ft_strlen(t_gstr_front("")) == 0, "expected 0");
	n = 16 << 20;
	g = t_gmem(NULL, n + 1);
	memset(g, 'x', n);
	g[n] = 0;
	CASE("ft_strlen(<16 MB string>)");
	EXPECT(ft_strlen(g) == n, "expected %zu, got %zu", n, ft_strlen(g));
}

/* ------------------------------------------------------------------ */
/* ft_memset / ft_bzero                                               */
/* ------------------------------------------------------------------ */

static void	test_memset(void)
{
	unsigned char	*g;
	unsigned char	buf[64];
	size_t			n;
	size_t			i;
	void			*r;
	int				ok;

	n = 0;
	while (n <= 130)
	{
		CASE("ft_memset(ptr, 'x', %zu) [guarded]", n);
		g = t_gmem(NULL, n);
		r = ft_memset(g, 'x', n);
		EXPECT(r == g, "must return its first argument");
		ok = 1;
		i = 0;
		while (i < n)
			ok &= (g[i++] == 'x');
		EXPECT(ok, "not every byte was set");
		CASE("ft_memset(ptr, 'x', %zu) [front guarded]", n);
		g = t_gmem_front(NULL, n);
		ft_memset(g, 'x', n);
		n++;
	}
	memset(buf, 'Z', sizeof(buf));
	CASE("ft_memset(buf + 8, 'x', 10) must not touch other bytes");
	ft_memset(buf + 8, 'x', 10);
	ok = 1;
	i = 0;
	while (i < sizeof(buf))
	{
		ok &= (buf[i] == ((i >= 8 && i < 18) ? 'x' : 'Z'));
		i++;
	}
	EXPECT(ok, "wrong bytes written");
	CASE("ft_memset(buf, 256 + 'A', 5) (c is converted to unsigned char)");
	ft_memset(buf, 256 + 'A', 5);
	EXPECT(memcmp(buf, "AAAAA", 5) == 0, "expected \"AAAAA\", got \"%s\"", t_escn((char *)buf, 5));
	CASE("ft_memset(buf, -1, 5)");
	ft_memset(buf, -1, 5);
	EXPECT(memcmp(buf, "\xff\xff\xff\xff\xff", 5) == 0, "expected 0xff bytes");
	CASE("ft_memset(buf, 'q', 0) changes nothing");
	memset(buf, 'Z', sizeof(buf));
	r = ft_memset(buf, 'q', 0);
	EXPECT(r == buf && buf[0] == 'Z', "wrote with n = 0 or wrong return");
	n = 16 << 20;
	g = t_gmem(NULL, n);
	CASE("ft_memset(<16 MB>, 0, 16 MB) [guarded]");
	ft_memset(g, 0, n);
	EXPECT(g[0] == 0 && g[n / 2] == 0 && g[n - 1] == 0, "not zeroed");
}

static void	test_bzero(void)
{
	unsigned char	*g;
	unsigned char	buf[32];
	size_t			n;
	size_t			i;
	int				ok;

	n = 0;
	while (n <= 130)
	{
		CASE("ft_bzero(ptr, %zu) [guarded]", n);
		g = t_gmem(NULL, n);
		ft_bzero(g, n);
		ok = 1;
		i = 0;
		while (i < n)
			ok &= (g[i++] == 0);
		EXPECT(ok, "not every byte was zeroed");
		CASE("ft_bzero(ptr, %zu) [front guarded]", n);
		ft_bzero(t_gmem_front(NULL, n), n);
		n++;
	}
	memset(buf, 'Z', sizeof(buf));
	CASE("ft_bzero(buf + 4, 3) must not touch other bytes");
	ft_bzero(buf + 4, 3);
	EXPECT(buf[3] == 'Z' && buf[4] == 0 && buf[6] == 0 && buf[7] == 'Z', "wrong bytes written");
	CASE("ft_bzero(buf, 0) changes nothing");
	ft_bzero(buf, 0);
	EXPECT(buf[0] == 'Z', "wrote with n = 0");
}

/* ------------------------------------------------------------------ */
/* ft_memcpy / ft_memmove                                             */
/* ------------------------------------------------------------------ */

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

static void	test_memcpy(void)
{
	unsigned char	pat[256];
	unsigned char	a[128];
	unsigned char	b[128];
	unsigned char	*src;
	unsigned char	*dst;
	size_t			n;
	size_t			off;
	void			*r;

	fill_pattern(pat, sizeof(pat), 3);
	n = 0;
	while (n <= 130)
	{
		CASE("ft_memcpy(dst, src, %zu) [both guarded]", n);
		src = t_gmem(pat, n);
		dst = t_gmem(NULL, n);
		r = ft_memcpy(dst, src, n);
		EXPECT(r == dst, "must return dst");
		EXPECT(memcmp(dst, pat, n) == 0, "copied bytes differ");
		CASE("ft_memcpy(dst, src, %zu) [both front guarded]", n);
		src = t_gmem_front(pat, n);
		dst = t_gmem_front(NULL, n);
		ft_memcpy(dst, src, n);
		EXPECT(memcmp(dst, pat, n) == 0, "copied bytes differ");
		n++;
	}
	off = 0;
	while (off < 16)
	{
		n = 0;
		while (n < 100)
		{
			CASE("ft_memcpy(a + %zu, pattern + 3, %zu) (unaligned)", off, n);
			memset(a, 0, sizeof(a));
			memset(b, 0, sizeof(b));
			memcpy(b + off, pat + 3, n);
			ft_memcpy(a + off, pat + 3, n);
			EXPECT(memcmp(a, b, sizeof(a)) == 0, "result differs from memcpy");
			n++;
		}
		off++;
	}
	CASE("ft_memcpy(dst, \"ab\\0cd\", 5) does not stop at '\\0'");
	memset(a, 'Z', 8);
	ft_memcpy(a, "ab\0cd", 5);
	EXPECT(memcmp(a, "ab\0cdZ", 6) == 0, "got \"%s\"", t_escn((char *)a, 6));
	n = 8 << 20;
	src = t_gmem(NULL, n);
	dst = t_gmem(NULL, n);
	fill_pattern(src, n, 11);
	CASE("ft_memcpy(<8 MB>) [guarded]");
	ft_memcpy(dst, src, n);
	EXPECT(memcmp(dst, src, n) == 0, "copied bytes differ");
}

static void	test_memmove(void)
{
	unsigned char	a[256];
	unsigned char	b[256];
	unsigned char	*g;
	unsigned char	*ref;
	size_t			so;
	size_t			d;
	size_t			n;
	void			*r;

	so = 0;
	while (so <= 24)
	{
		d = 0;
		while (d <= 24)
		{
			n = 0;
			while (n <= 48)
			{
				CASE("ft_memmove(buf + %zu, buf + %zu, %zu) (overlapping)", d, so, n);
				fill_pattern(a, sizeof(a), 5);
				fill_pattern(b, sizeof(b), 5);
				memmove(b + d, b + so, n);
				r = ft_memmove(a + d, a + so, n);
				EXPECT(r == a + d, "must return dst");
				EXPECT(memcmp(a, b, sizeof(a)) == 0, "result differs from memmove");
				n++;
			}
			d++;
		}
		so++;
	}
	n = 0;
	while (n <= 64)
	{
		CASE("ft_memmove(g + 1, g, %zu) [g guarded, overlap reaches the guard]", n);
		g = t_gmem(NULL, n + 1);
		fill_pattern(g, n + 1, 1);
		ref = t_gmem(g, n + 1);
		memmove(ref + 1, ref, n);
		ft_memmove(g + 1, g, n);
		EXPECT(memcmp(g, ref, n + 1) == 0, "result differs from memmove");
		CASE("ft_memmove(g, g + 1, %zu) [front guarded]", n);
		g = t_gmem_front(NULL, n + 1);
		fill_pattern(g, n + 1, 1);
		ref = t_gmem(g, n + 1);
		memmove(ref, ref + 1, n);
		ft_memmove(g, g + 1, n);
		EXPECT(memcmp(g, ref, n + 1) == 0, "result differs from memmove");
		CASE("ft_memmove(dst, src, %zu) [separate guarded buffers]", n);
		g = t_gmem(NULL, n);
		ref = t_gmem(a, n);
		ft_memmove(g, ref, n);
		EXPECT(memcmp(g, a, n) == 0, "copied bytes differ");
		n++;
	}
	CASE("ft_memmove(buf, buf, 10) (same pointer)");
	fill_pattern(a, 16, 9);
	fill_pattern(b, 16, 9);
	EXPECT(ft_memmove(a, a, 10) == a && memcmp(a, b, 16) == 0, "buffer changed or wrong return");
	n = 8 << 20;
	g = t_gmem(NULL, n);
	ref = t_gmem(NULL, n);
	fill_pattern(g, n, 3);
	fill_pattern(ref, n, 3);
	CASE("ft_memmove(big + 1, big, 8MB - 1) [guarded]");
	memmove(ref + 1, ref, n - 1);
	ft_memmove(g + 1, g, n - 1);
	EXPECT(memcmp(g, ref, n) == 0, "result differs from memmove");
	CASE("ft_memmove(big, big + 1, 8MB - 1) [guarded]");
	memmove(ref, ref + 1, n - 1);
	ft_memmove(g, g + 1, n - 1);
	EXPECT(memcmp(g, ref, n) == 0, "result differs from memmove");
}

static void	test_mem_null(void)
{
	CASE("ft_memcpy(NULL, NULL, 0)");
	ft_memcpy(NULL, NULL, 0);
	CASE("ft_memmove(NULL, NULL, 0)");
	ft_memmove(NULL, NULL, 0);
	CASE("ft_memcpy(NULL, NULL, 5)");
	EXPECT(ft_memcpy(NULL, NULL, 5) == NULL, "should return NULL");
	CASE("ft_memmove(NULL, NULL, 5)");
	EXPECT(ft_memmove(NULL, NULL, 5) == NULL, "should return NULL");
}

/* ------------------------------------------------------------------ */
/* ft_strlcpy / ft_strlcat                                            */
/* ------------------------------------------------------------------ */

static void	test_strlcpy(void)
{
	const char	*srcs[] = {"", "a", "hello", "hello world 42 !",
		"lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod"};
	char		ref[128];
	char		*dst;
	char		*src;
	size_t		i;
	size_t		size;
	size_t		len;
	size_t		r;

	i = 0;
	while (i < sizeof(srcs) / sizeof(*srcs))
	{
		len = strlen(srcs[i]);
		src = t_gstr(srcs[i]);
		size = 0;
		while (size <= len + 3)
		{
			CASE("ft_strlcpy(dst[%zu], \"%s\", %zu) [guarded]", size, t_esc(srcs[i]), size);
			dst = t_gmem(NULL, size);
			memset(ref, 0xAA, sizeof(ref));
			ref_strlcpy(ref, srcs[i], size);
			r = ft_strlcpy(dst, src, size);
			EXPECT(r == len, "expected return %zu, got %zu", len, r);
			EXPECT(memcmp(dst, ref, size) == 0, "expected \"%s\", got \"%s\"",
				t_escn(ref, size), t_escn(dst, size));
			size++;
		}
		CASE("ft_strlcpy(dst[%zu], \"%s\", SIZE_MAX) [guarded]", len + 1, t_esc(srcs[i]));
		dst = t_gmem(NULL, len + 1);
		r = ft_strlcpy(dst, src, SIZE_MAX);
		EXPECT(r == len && strcmp(dst, srcs[i]) == 0, "wrong copy or return (%zu)", r);
		i++;
	}
}

static void	test_strlcat(void)
{
	const char	*dsts[] = {"", "ab", "hello", "lorem ipsum"};
	const char	*srcs[] = {"", "x", "world!!", "dolor sit amet"};
	char		ref[128];
	char		*dst;
	char		*src;
	size_t		i;
	size_t		j;
	size_t		size;
	size_t		dl;
	size_t		exp;
	size_t		r;

	i = 0;
	while (i < sizeof(dsts) / sizeof(*dsts))
	{
		j = 0;
		while (j < sizeof(srcs) / sizeof(*srcs))
		{
			dl = strlen(dsts[i]);
			src = t_gstr(srcs[j]);
			size = 0;
			while (size <= dl + strlen(srcs[j]) + 3)
			{
				CASE("ft_strlcat(dst[%zu] = \"%s\", \"%s\", %zu) [guarded%s]", size,
					t_esc(dsts[i]), t_esc(srcs[j]), size,
					size <= dl ? ", dst has NO '\\0' inside size" : "");
				memset(ref, 0xAA, sizeof(ref));
				memcpy(ref, dsts[i], dl + 1 < size ? dl + 1 : size);
				dst = t_gmem(ref, size);
				exp = ref_strlcat(ref, srcs[j], size);
				r = ft_strlcat(dst, src, size);
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

static void	test_strl_null(void)
{
	CASE("ft_strlcpy(NULL, \"abc\", 0)");
	EXPECT(ft_strlcpy(NULL, "abc", 0) == 3, "should return 3");
	CASE("ft_strlcat(NULL, \"abc\", 0)");
	EXPECT(ft_strlcat(NULL, "abc", 0) == 3, "should return 3 (BSD reads nothing from dst when size is 0)");
}

/* ------------------------------------------------------------------ */
/* ft_strchr / ft_strrchr                                             */
/* ------------------------------------------------------------------ */

static void	check_chr(const char *name, char *(*ft)(const char *, int),
	char *(*ref)(const char *, int), const char *s, int front)
{
	char	*g;
	size_t	len;
	size_t	i;
	int		cs[4];
	int		k;
	char	*got;
	char	*exp;

	g = front ? t_gstr_front(s) : t_gstr(s);
	len = strlen(s);
	i = 0;
	while (i <= len + 1)
	{
		cs[0] = i <= len ? (unsigned char)s[i] : 'z';
		cs[1] = (char)cs[0];
		cs[2] = cs[0] + 256;
		cs[3] = i <= len ? cs[0] - 256 : '~';
		k = 0;
		while (k < 4)
		{
			CASE("%s(\"%s\", %d) [%s]", name, t_esc(s), cs[k], front ? "front guarded" : "guarded");
			got = ft(g, cs[k]);
			exp = ref(s, cs[k]);
			if (!exp)
				EXPECT(got == NULL, "expected NULL, got pointer to \"%s\"", t_esc(got));
			else
				EXPECT(got == g + (exp - s), "expected pointer to index %td, got %s%td", exp - s,
					got ? "index " : "NULL ", got ? got - g : (ptrdiff_t)0);
			k++;
		}
		i++;
	}
}

static char	*w_strchr(const char *s, int c) { return ((char *)strchr(s, c)); }
static char	*w_strrchr(const char *s, int c) { return ((char *)strrchr(s, c)); }

static void	test_strchr(void)
{
	const char	*strs[] = {"", "a", "Hello, World!", "abcabc", "tripouille\xc8\x80 end", "\xff\x01\x7f"};
	size_t		i;

	i = 0;
	while (i < sizeof(strs) / sizeof(*strs))
	{
		check_chr("ft_strchr", ft_strchr, w_strchr, strs[i], 0);
		check_chr("ft_strchr", ft_strchr, w_strchr, strs[i], 1);
		i++;
	}
}

static void	test_strrchr(void)
{
	const char	*strs[] = {"", "a", "Hello, World!", "abcabc", "tripouille\xc8\x80 end", "\xff\x01\x7f", "aaaa"};
	size_t		i;

	i = 0;
	while (i < sizeof(strs) / sizeof(*strs))
	{
		check_chr("ft_strrchr", ft_strrchr, w_strrchr, strs[i], 0);
		check_chr("ft_strrchr", ft_strrchr, w_strrchr, strs[i], 1);
		i++;
	}
}

/* ------------------------------------------------------------------ */
/* ft_strncmp / ft_memcmp / ft_memchr                                 */
/* ------------------------------------------------------------------ */

static void	test_strncmp(void)
{
	const char	*p[][2] = {{"abc", "abc"}, {"abc", "abd"}, {"abd", "abc"}, {"abc", "ab"},
		{"ab", "abc"}, {"", ""}, {"", "a"}, {"a", ""}, {"\200", "\0"}, {"\0", "\200"},
		{"\xff", "\x01"}, {"test\200", "test\0"}, {"abcdef", "abcxyz"}, {"1234", "1235"}};
	size_t		ns[] = {0, 1, 2, 3, 4, 5, 6, 10, 42, SIZE_MAX};
	size_t		i;
	size_t		k;
	int			exp;
	int			got;

	i = 0;
	while (i < sizeof(p) / sizeof(*p))
	{
		k = 0;
		while (k < sizeof(ns) / sizeof(*ns))
		{
			CASE("ft_strncmp(\"%s\", \"%s\", %zu) [guarded]", t_esc(p[i][0]), t_esc(p[i][1]), ns[k]);
			exp = sign(strncmp(p[i][0], p[i][1], ns[k]));
			got = sign(ft_strncmp(t_gstr(p[i][0]), t_gstr(p[i][1]), ns[k]));
			EXPECT(got == exp, "expected a result %s 0 (compare as unsigned char), got %s 0",
				exp < 0 ? "<" : exp > 0 ? ">" : "==", got < 0 ? "<" : got > 0 ? ">" : "==");
			CASE("ft_strncmp(\"%s\", \"%s\", %zu) [front guarded]", t_esc(p[i][0]), t_esc(p[i][1]), ns[k]);
			got = sign(ft_strncmp(t_gstr_front(p[i][0]), t_gstr_front(p[i][1]), ns[k]));
			EXPECT(got == exp, "wrong sign");
			k++;
		}
		i++;
	}
}

static void	test_memcmp(void)
{
	const char	*p[][2] = {{"abc", "abc"}, {"abc", "abd"}, {"abd", "abc"}, {"\x80", "\x01"},
		{"\x01", "\x80"}, {"a\0b", "a\0c"}, {"a\0c", "a\0b"}, {"\xff\xff", "\xff\xfe"}, {"t\200", "t\0"}};
	size_t		lens[] = {3, 3, 3, 1, 1, 3, 3, 2, 2};
	size_t		i;
	size_t		n;
	int			exp;
	int			got;

	i = 0;
	while (i < sizeof(p) / sizeof(*p))
	{
		n = 0;
		while (n <= lens[i])
		{
			CASE("ft_memcmp(\"%s\", \"%s\", %zu) [guarded]", t_escn(p[i][0], lens[i]),
				t_escn(p[i][1], lens[i]), n);
			exp = sign(memcmp(p[i][0], p[i][1], n));
			got = sign(ft_memcmp(t_gmem(p[i][0], n), t_gmem(p[i][1], n), n));
			EXPECT(got == exp, "expected a result %s 0, got %s 0 (bytes are unsigned char, '\\0' is not special)",
				exp < 0 ? "<" : exp > 0 ? ">" : "==", got < 0 ? "<" : got > 0 ? ">" : "==");
			n++;
		}
		i++;
	}
}

static void	test_memchr(void)
{
	const char	buf[] = "abc\0def\xc8";
	char		*g;
	int			cs[] = {'a', 'c', 0, 'd', 0xc8, -56, 'd' + 256, 'z', 'f'};
	size_t		n;
	size_t		k;
	const void	*exp;
	void		*got;

	g = t_gmem(buf, 8);
	k = 0;
	while (k < sizeof(cs) / sizeof(*cs))
	{
		n = 0;
		while (n <= 8)
		{
			CASE("ft_memchr(\"abc\\0def\\xc8\", %d, %zu) [guarded]", cs[k], n);
			exp = memchr(buf, cs[k], n);
			got = ft_memchr(g, cs[k], n);
			if (!exp)
				EXPECT(got == NULL, "expected NULL, got index %td", (char *)got - g);
			else
				EXPECT(got == g + ((const char *)exp - buf), "expected index %td, got %s",
					(const char *)exp - buf, got ? "another pointer" : "NULL");
			n++;
		}
		k++;
	}
}

static void	test_memchr_c11(void)
{
	char	*g;

	g = t_gmem("abcx", 4);
	CASE("ft_memchr(<4 bytes ending in 'x'>, 'x', SIZE_MAX) [guarded]");
	EXPECT(ft_memchr(g, 'x', SIZE_MAX) == g + 3, "should stop at the first match (C11 7.24.5.1)");
}

/* ------------------------------------------------------------------ */
/* ft_strnstr                                                         */
/* ------------------------------------------------------------------ */

static void	test_strnstr(void)
{
	const char	*bigs[] = {"", "a", "aaab", "aaaaab", "Hello World", "lorem ipsum dolor sit amet", "MZIRIBMZIRIBMZE123"};
	const char	*lits[] = {"", "a", "b", "ab", "aab", "World", "dolor", "lorem ipsum dolor sit amet!",
		"o", "MZIRIBMZE", "d", "lorem"};
	size_t		lens[] = {0, 1, 2, 3, 4, 5, 6, 7, 10, 11, 12, 15, 17, 30, SIZE_MAX};
	size_t		i;
	size_t		j;
	size_t		k;
	char		*gb;
	char		*gl;
	const char	*exp;
	char		*got;

	i = 0;
	while (i < sizeof(bigs) / sizeof(*bigs))
	{
		gb = t_gstr(bigs[i]);
		j = 0;
		while (j < sizeof(lits) / sizeof(*lits))
		{
			gl = t_gstr(lits[j]);
			k = 0;
			while (k < sizeof(lens) / sizeof(*lens))
			{
				CASE("ft_strnstr(\"%s\", \"%s\", %zu) [guarded]", t_esc(bigs[i]), t_esc(lits[j]), lens[k]);
				exp = ref_strnstr(bigs[i], lits[j], lens[k]);
				got = ft_strnstr(gb, gl, lens[k]);
				if (!exp)
					EXPECT(got == NULL, "expected NULL, got \"%s\"", t_esc(got));
				else
					EXPECT(got == gb + (exp - bigs[i]), "expected pointer to \"%s\", got %s\"%s\"",
						t_esc(exp), got ? "" : "NULL ", t_esc(got));
				k++;
			}
			j++;
		}
		i++;
	}
}

static void	test_strnstr_null(void)
{
	CASE("ft_strnstr(NULL, \"abc\", 0)");
	EXPECT(ft_strnstr(NULL, "abc", 0) == NULL, "should return NULL");
}

/* ------------------------------------------------------------------ */
/* ft_atoi                                                            */
/* ------------------------------------------------------------------ */

static void	test_atoi(void)
{
	const char	*strs[] = {"0", "42", "-42", "+42", " 42", "\t\n\v\f\r 42", "   -42abc",
		"+-42", "-+42", "--42", "++42", "+ 42", "- 42", " - 42", "42abc", "abc42", "", " ",
		"-", "+", "-0", "+0", "00000000000000000000000000042", "-0000000000000000000000000042",
		"2147483647", "-2147483648", "-2147483647", "2147483646", "1\n2", "\x80 42", "\x1b 42",
		"0x1A", "12 34", "\n\n\n  -46\b9 \n5d6", " \t\v\f\r\n  +0012a", "\xe2\x80\x83 1",
		"9", "-9", "10", "-10", "1000000000", "-1000000000", "\x7f""42", "4\x00""2"};
	size_t		i;
	int			exp;
	int			got;

	i = 0;
	while (i < sizeof(strs) / sizeof(*strs))
	{
		CASE("ft_atoi(\"%s\") [guarded]", t_esc(strs[i]));
		exp = atoi(strs[i]);
		got = ft_atoi(t_gstr(strs[i]));
		EXPECT(got == exp, "expected %d, got %d", exp, got);
		CASE("ft_atoi(\"%s\") [front guarded]", t_esc(strs[i]));
		got = ft_atoi(t_gstr_front(strs[i]));
		EXPECT(got == exp, "expected %d, got %d", exp, got);
		i++;
	}
}

static void	test_atoi_overflow(void)
{
	const char	*strs[] = {"2147483648", "-2147483649", "9999999999", "99999999999999999999",
		"-99999999999999999999", "18446744073709551616", "9223372036854775808"};
	size_t		i;

	i = 0;
	while (i < sizeof(strs) / sizeof(*strs))
	{
		CASE("ft_atoi(\"%s\") (overflow is UB, it only must not crash)", strs[i]);
		(void)ft_atoi(t_gstr(strs[i]));
		i++;
	}
}

/* ------------------------------------------------------------------ */
/* ft_calloc / ft_strdup                                              */
/* ------------------------------------------------------------------ */

static void	check_calloc_zeroed(size_t nmemb, size_t size)
{
	unsigned char	*p;
	size_t			i;
	int				ok;

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
	ok = 1;
	i = 0;
	while (i < nmemb * size)
		ok &= (p[i++] == 0);
	EXPECT(ok, "memory is not zeroed (the tester's malloc returns garbage on purpose)");
	free(p);
}

static void	test_calloc(void)
{
	check_calloc_zeroed(5, 4);
	check_calloc_zeroed(1, 1);
	check_calloc_zeroed(42, 1);
	check_calloc_zeroed(1, 42);
	check_calloc_zeroed(3, sizeof(long));
	check_calloc_zeroed(1000, 1000);
	check_calloc_zeroed(4 << 20, 8);
}

static void	test_calloc_zero(void)
{
	void	*p[3];
	size_t	args[3][2] = {{0, 0}, {0, 5}, {5, 0}};
	int		i;

	i = 0;
	while (i < 3)
	{
		CASE("ft_calloc(%zu, %zu)", args[i][0], args[i][1]);
		p[i] = ft_calloc(args[i][0], args[i][1]);
		EXPECT(p[i] != NULL, "subject: must return a unique pointer that can be passed to free(), got NULL");
		EXPECT(!p[i] || t_is_block(p[i]), "the returned pointer can't be passed to free()");
		i++;
	}
	CASE("ft_calloc(0, 0) three times");
	EXPECT(!p[0] || !p[1] || (p[0] != p[1] && p[1] != p[2] && p[0] != p[2]),
		"the pointers are not unique");
	t_free(p[0]);
	t_free(p[1]);
	t_free(p[2]);
}

static void	test_calloc_overflow(void)
{
	size_t	args[][2] = {{SIZE_MAX / 2 + 1, 2}, {SIZE_MAX, SIZE_MAX}, {SIZE_MAX, 2},
		{2, SIZE_MAX}, {(size_t)1 << 32, (size_t)1 << 32}, {(size_t)1 << 33, (size_t)1 << 31},
		{65536, 65537}, {1, SIZE_MAX}, {SIZE_MAX, 1}, {(size_t)-10, sizeof(int)}};
	size_t	i;
	void	*p;

	i = 0;
	while (i < sizeof(args) / sizeof(*args))
	{
		CASE("ft_calloc(%zu, %zu)", args[i][0], args[i][1]);
		p = ft_calloc(args[i][0], args[i][1]);
		EXPECT(p == NULL, "expected NULL (nmemb * size overflows / is way too big), got a %zu-byte block",
			t_block_size(p));
		t_free(p);
		i++;
	}
}

static void	test_strdup(void)
{
	const char	*strs[] = {"", "a", "hello", "\xff\x80\x01", "lorem ipsum dolor sit amet"};
	size_t		i;
	char		*g;
	char		*r;
	size_t		n;

	i = 0;
	while (i < sizeof(strs) / sizeof(*strs))
	{
		CASE("ft_strdup(\"%s\") [guarded]", t_esc(strs[i]));
		g = t_gstr(strs[i]);
		r = ft_strdup(g);
		EXPECT(r != g, "must return a new copy, not its argument");
		t_check_str("ft_strdup", r, strs[i]);
		CASE("ft_strdup(\"%s\") [front guarded]", t_esc(strs[i]));
		t_check_str("ft_strdup", ft_strdup(t_gstr_front(strs[i])), strs[i]);
		i++;
	}
	n = 4 << 20;
	g = t_gmem(NULL, n + 1);
	memset(g, 'k', n);
	g[n] = 0;
	CASE("ft_strdup(<4 MB string>) [guarded]");
	r = ft_strdup(g);
	EXPECT(r && t_block_size(r) >= n + 1 && memcmp(r, g, n + 1) == 0, "wrong copy");
	t_free(r);
}

/* ------------------------------------------------------------------ */

void	run_part1(void)
{
	t_section("PART 1 - LIBC FUNCTIONS");
	t_run("ft_isalpha (-1..255, must return 1 or 0)", test_isalpha, T_MUST, 5);
	t_run("ft_isdigit (-1..255, must return 1 or 0)", test_isdigit, T_MUST, 5);
	t_run("ft_isalnum (-1..255, must return 1 or 0)", test_isalnum, T_MUST, 5);
	t_run("ft_isascii (-1..255, must return 1 or 0)", test_isascii, T_MUST, 5);
	t_run("ft_isprint (-1..255, must return 1 or 0)", test_isprint, T_MUST, 5);
	t_run("ft_is* outside unsigned char (UB)", test_is_outside, T_WARN, 5);
	t_run("ft_toupper / ft_tolower (-1..255)", test_toupper_tolower, T_MUST, 5);
	t_run("ft_toupper / ft_tolower outside unsigned char (UB)", test_toupper_tolower_outside, T_WARN, 5);
	t_run("ft_strlen", test_strlen, T_MUST, 5);
	t_run("ft_memset", test_memset, T_MUST, 5);
	t_run("ft_bzero", test_bzero, T_MUST, 5);
	t_run("ft_memcpy", test_memcpy, T_MUST, 5);
	t_run("ft_memmove (all overlaps vs libc)", test_memmove, T_MUST, 10);
	t_run("ft_memcpy / ft_memmove with NULL (UB)", test_mem_null, T_WARN, 5);
	t_run("ft_strlcpy", test_strlcpy, T_MUST, 5);
	t_run("ft_strlcat (incl. dst without '\\0' inside size)", test_strlcat, T_MUST, 5);
	t_run("ft_strlcpy / ft_strlcat with NULL dst and size 0", test_strl_null, T_WARN, 5);
	t_run("ft_strchr (incl. c > 255, c = '\\0', chars > 127)", test_strchr, T_MUST, 5);
	t_run("ft_strrchr (incl. c > 255, c = '\\0', chars > 127)", test_strrchr, T_MUST, 5);
	t_run("ft_strncmp (unsigned, n = 0 .. SIZE_MAX)", test_strncmp, T_MUST, 5);
	t_run("ft_memchr", test_memchr, T_MUST, 5);
	t_run("ft_memchr stops at the first match (n = SIZE_MAX)", test_memchr_c11, T_WARN, 5);
	t_run("ft_memcmp", test_memcmp, T_MUST, 5);
	t_run("ft_strnstr", test_strnstr, T_MUST, 10);
	t_run("ft_strnstr(NULL, \"abc\", 0) (like BSD)", test_strnstr_null, T_WARN, 5);
	t_run("ft_atoi", test_atoi, T_MUST, 5);
	t_run("ft_atoi overflow must not crash (UB)", test_atoi_overflow, T_WARN, 5);
	t_run("ft_calloc (memory really zeroed, from malloc)", test_calloc, T_MUST, 10);
	t_run("ft_calloc(0, x) / (x, 0) (unique freeable pointer)", test_calloc_zero, T_MUST, 5);
	t_run("ft_calloc overflow of nmemb * size", test_calloc_overflow, T_MUST, 5);
	t_run("ft_strdup", test_strdup, T_MUST, 5);
}
