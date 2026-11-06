#include "main.h"

static int	same_bool(int a, int b) { return ((a != 0) == (b != 0)); }
static int	sgn(int x) { return ((x > 0) - (x < 0)); }

/* free la chaine et compare avec l'attendu */
static int	streq_free(char *s, const char *expected)
{
	int	r;

	r = (s != NULL && strcmp(s, expected) == 0);
	free(s);
	return (r);
}

/* ===== Versions de reference (absentes de la glibc selon la version) ===== */

static size_t	ref_strlcpy(char *dst, const char *src, size_t size)
{
	size_t	l;
	size_t	n;

	l = strlen(src);
	if (size)
	{
		n = (l >= size) ? size - 1 : l;
		memcpy(dst, src, n);
		dst[n] = '\0';
	}
	return (l);
}

static size_t	ref_strlcat(char *dst, const char *src, size_t size)
{
	size_t	dl;
	size_t	sl;
	size_t	room;
	size_t	n;

	dl = 0;
	while (dl < size && dst[dl])
		dl++;
	sl = strlen(src);
	if (dl == size)
		return (size + sl);
	room = size - dl - 1;
	n = (sl < room) ? sl : room;
	memcpy(dst + dl, src, n);
	dst[dl + n] = '\0';
	return (dl + sl);
}

static char	*ref_strnstr(const char *big, const char *little, size_t len)
{
	size_t	i;
	size_t	ll;

	if (!*little)
		return ((char *)big);
	ll = strlen(little);
	i = 0;
	while (i < len && big[i])
	{
		if (i + ll > len)
			break ;
		if (strncmp(big + i, little, ll) == 0)
			return ((char *)big + i);
		i++;
	}
	return (NULL);
}

/* ========================================================================== */
/*                                   CTYPE                                    */
/* ========================================================================== */

/* compare libc et ft sur tout l'intervalle valide : EOF (-1) et 0..255 */
#define CTYPE_TEST(fn) \
static void	fn##_test(void) \
{ \
	int	i; \
	int	bad; \
	\
	START("ft_" #fn); \
	bad = -2; \
	i = -1; \
	while (i <= 255) \
	{ \
		if (!same_bool(fn(i), ft_##fn(i)) && bad == -2) \
			bad = i; \
		i++; \
	} \
	if (bad == -2) \
		printf("OK (-1..255)"); \
	else \
		{ printf("KO (premier ecart : c = %d)", bad); g_fail++; } \
	END(); \
}

CTYPE_TEST(isalpha)
CTYPE_TEST(isdigit)
CTYPE_TEST(isalnum)
CTYPE_TEST(isascii)
CTYPE_TEST(isprint)

/* tolower / toupper : la valeur retournee doit etre identique */
#define CASE_TEST(fn) \
static void	fn##_test(void) \
{ \
	int	i; \
	int	bad; \
	\
	START("ft_" #fn); \
	bad = -2; \
	i = -1; \
	while (i <= 255) \
	{ \
		if (fn(i) != ft_##fn(i) && bad == -2) \
			bad = i; \
		i++; \
	} \
	if (bad == -2) \
		printf("OK (-1..255)"); \
	else \
		{ printf("KO (premier ecart : c = %d)", bad); g_fail++; } \
	END(); \
}

CASE_TEST(tolower)
CASE_TEST(toupper)

/* ========================================================================== */
/*                                 STRING UTILS                               */
/* ========================================================================== */

static void	strlen_test(void)
{
	START("ft_strlen");
	T(strlen("") == ft_strlen(""));
	T(strlen("a") == ft_strlen("a"));
	T(strlen("Bonjour") == ft_strlen("Bonjour"));
	T(strlen("a\0b") == ft_strlen("a\0b"));
	T(strlen("   \t\n  ") == ft_strlen("   \t\n  "));
	T(strlen("0123456789012345678901234567890123456789")
		== ft_strlen("0123456789012345678901234567890123456789"));
	END();
}

static void	t_strlcpy(const char *src, size_t size)
{
	char	d[32];
	char	f[32];
	size_t	r1;
	size_t	r2;

	memset(d, 0x55, sizeof(d));
	memset(f, 0x55, sizeof(f));
	r1 = ref_strlcpy(d, src, size);
	r2 = ft_strlcpy(f, src, size);
	T(r1 == r2 && memcmp(d, f, sizeof(d)) == 0);
}

static void	strlcpy_test(void)
{
	START("ft_strlcpy");
	t_strlcpy("Bonjour", 8);
	t_strlcpy("Bonjour", 0);
	t_strlcpy("Bonjour", 1);
	t_strlcpy("Bonjour", 4);
	t_strlcpy("Bonjour", 7);
	t_strlcpy("Bonjour", 20);
	t_strlcpy("", 5);
	t_strlcpy("", 0);
	END();
}

static void	t_strlcat(const char *start, const char *src, size_t size)
{
	char	d[32];
	char	f[32];
	size_t	r1;
	size_t	r2;

	memset(d, 0x55, sizeof(d));
	memset(f, 0x55, sizeof(f));
	strcpy(d, start);
	strcpy(f, start);
	r1 = ref_strlcat(d, src, size);
	r2 = ft_strlcat(f, src, size);
	T(r1 == r2 && memcmp(d, f, sizeof(d)) == 0);
}

static void	strlcat_test(void)
{
	START("ft_strlcat");
	t_strlcat("Hello ", "World", 20);
	t_strlcat("Hello ", "World", 12);
	t_strlcat("Hello ", "World", 11);
	t_strlcat("Hello ", "World", 8);
	t_strlcat("Hello ", "World", 6);
	t_strlcat("Hello ", "World", 3);
	t_strlcat("Hello ", "World", 0);
	t_strlcat("", "World", 10);
	t_strlcat("Hello", "", 10);
	t_strlcat("", "", 1);
	END();
}

static void	strncmp_test(void)
{
	START("ft_strncmp");
	T(sgn(strncmp("abc", "abc", 3)) == sgn(ft_strncmp("abc", "abc", 3)));
	T(sgn(strncmp("abc", "abd", 3)) == sgn(ft_strncmp("abc", "abd", 3)));
	T(sgn(strncmp("abd", "abc", 3)) == sgn(ft_strncmp("abd", "abc", 3)));
	T(sgn(strncmp("abc", "abd", 2)) == sgn(ft_strncmp("abc", "abd", 2)));
	T(sgn(strncmp("abc", "abd", 0)) == sgn(ft_strncmp("abc", "abd", 0)));
	T(sgn(strncmp("abc", "abcd", 10)) == sgn(ft_strncmp("abc", "abcd", 10)));
	T(sgn(strncmp("abcd", "abc", 10)) == sgn(ft_strncmp("abcd", "abc", 10)));
	T(sgn(strncmp("", "", 5)) == sgn(ft_strncmp("", "", 5)));
	T(sgn(strncmp("", "a", 5)) == sgn(ft_strncmp("", "a", 5)));
	T(sgn(strncmp("test\200", "test\0", 6)) == sgn(ft_strncmp("test\200", "test\0", 6)));
	T(sgn(strncmp("\200", "\0", 1)) == sgn(ft_strncmp("\200", "\0", 1)));
	END();
}

static void	strdup_test(void)
{
	char	*a;
	char	*b;
	char	src[] = "Bonjour 42";

	START("ft_strdup");
	a = strdup(src);
	b = ft_strdup(src);
	T(b != NULL && b != src && strcmp(a, b) == 0);
	free(a);
	free(b);
	a = strdup("");
	b = ft_strdup("");
	T(b != NULL && strcmp(a, b) == 0);
	free(a);
	free(b);
	b = ft_strdup(src);
	src[0] = 'X';
	T(b != NULL && b[0] == 'B');
	free(b);
	END();
}

static void	strchr_test(void)
{
	const char	*s = "Hello, 42 school";

	START("ft_strchr");
	T(strchr(s, 'H') == ft_strchr(s, 'H'));
	T(strchr(s, 'l') == ft_strchr(s, 'l'));
	T(strchr(s, 'l' + 256) == ft_strchr(s, 'l' + 256));
	T(strchr(s, 'o') == ft_strchr(s, 'o'));
	T(strchr(s, 'z') == ft_strchr(s, 'z'));
	T(strchr(s, '\0') == ft_strchr(s, '\0'));
	T(strchr("", '\0') == ft_strchr("", '\0'));
	T(strchr("", 'a') == ft_strchr("", 'a'));
	END();
}

static void	strrchr_test(void)
{
	const char	*s = "Hello, 42 school";

	START("ft_strrchr");
	T(strrchr(s, 'H') == ft_strrchr(s, 'H'));
	T(strrchr(s, 'l') == ft_strrchr(s, 'l'));
	T(strrchr(s, 'o') == ft_strrchr(s, 'o'));
	T(strrchr(s, 'z') == ft_strrchr(s, 'z'));
	T(strrchr(s, '\0') == ft_strrchr(s, '\0'));
	T(strrchr("", '\0') == ft_strrchr("", '\0'));
	T(strrchr("", 'a') == ft_strrchr("", 'a'));
	END();
}

static void	strnstr_test(void)
{
	const char	*big = "Foo Bar Baz";

	START("ft_strnstr");
	T(ref_strnstr(big, "Bar", 11) == ft_strnstr(big, "Bar", 11));
	T(ref_strnstr(big, "Bar", 7) == ft_strnstr(big, "Bar", 7));
	T(ref_strnstr(big, "Bar", 6) == ft_strnstr(big, "Bar", 6));
	T(ref_strnstr(big, "Baz", 11) == ft_strnstr(big, "Baz", 11));
	T(ref_strnstr(big, "Baz", 10) == ft_strnstr(big, "Baz", 10));
	T(ref_strnstr(big, "Foo", 0) == ft_strnstr(big, "Foo", 0));
	T(ref_strnstr(big, "", 0) == ft_strnstr(big, "", 0));
	T(ref_strnstr(big, "", 5) == ft_strnstr(big, "", 5));
	T(ref_strnstr(big, "xyz", 11) == ft_strnstr(big, "xyz", 11));
	T(ref_strnstr("", "a", 5) == ft_strnstr("", "a", 5));
	T(ref_strnstr("", "", 5) == ft_strnstr("", "", 5));
	T(ref_strnstr(big, "Foo Bar Baz Qux", 20) == ft_strnstr(big, "Foo Bar Baz Qux", 20));
	T(ref_strnstr("aaabc", "abc", 5) == ft_strnstr("aaabc", "abc", 5));
	END();
}

static void	atoi_test(void)
{
	const char	*tab[] = {"0", "42", "-42", "+42", "   42", "\t\n\v\f\r 42",
		"42abc", "abc", "", " ", "-", "+", "--1", "+-1", "-+1", "- 1",
		"2147483647", "-2147483648", "0042", "-0", "  -12  34", "12\0003", NULL};
	int			i;

	START("ft_atoi");
	i = 0;
	while (tab[i])
	{
		T(atoi(tab[i]) == ft_atoi(tab[i]));
		i++;
	}
	END();
}

/* ========================================================================== */
/*                                   MEMORY                                   */
/* ========================================================================== */

static void	bzero_test(void)
{
	char	a[10];
	char	b[10];

	START("ft_bzero");
	memset(a, 'X', 10);
	memset(b, 'X', 10);
	memset(a + 2, 0, 5);
	ft_bzero(b + 2, 5);
	T(memcmp(a, b, 10) == 0);
	memset(a, 'X', 10);
	memset(b, 'X', 10);
	ft_bzero(b, 0);
	T(memcmp(a, b, 10) == 0);
	memset(a, 0, 10);
	ft_bzero(b, 10);
	T(memcmp(a, b, 10) == 0);
	END();
}

static void	memset_test(void)
{
	char	a[10];
	char	b[10];

	START("ft_memset");
	memset(a, 'X', 10);
	memset(b, 'X', 10);
	memset(a + 1, 42, 5);
	T(ft_memset(b + 1, 42, 5) == b + 1 && memcmp(a, b, 10) == 0);
	memset(a, 'X', 10);
	memset(b, 'X', 10);
	memset(a, 256 + 65, 10);
	ft_memset(b, 256 + 65, 10);
	T(memcmp(a, b, 10) == 0);
	memset(a, 'X', 10);
	memset(b, 'X', 10);
	memset(a, 0, 0);
	ft_memset(b, 0, 0);
	T(memcmp(a, b, 10) == 0);
	END();
}

static void	memcpy_test(void)
{
	char	a[20];
	char	b[20];

	START("ft_memcpy");
	memset(a, 'X', 20);
	memset(b, 'X', 20);
	memcpy(a, "Bonjour 42", 11);
	T(ft_memcpy(b, "Bonjour 42", 11) == b && memcmp(a, b, 20) == 0);
	memset(a, 'X', 20);
	memset(b, 'X', 20);
	memcpy(a + 3, "abc", 0);
	ft_memcpy(b + 3, "abc", 0);
	T(memcmp(a, b, 20) == 0);
	memset(a, 'X', 20);
	memset(b, 'X', 20);
	memcpy(a + 5, "a\0b\0c", 5);
	ft_memcpy(b + 5, "a\0b\0c", 5);
	T(memcmp(a, b, 20) == 0);
	END();
}

static void	memmove_test(void)
{
	char	a[20] = {0};
	char	b[20] = {0};

	START("ft_memmove");
	strcpy(a, "0123456789");
	strcpy(b, "0123456789");
	memmove(a + 2, a, 5);
	T(ft_memmove(b + 2, b, 5) == b + 2 && memcmp(a, b, 20) == 0);
	strcpy(a, "0123456789");
	strcpy(b, "0123456789");
	memmove(a, a + 2, 5);
	T(ft_memmove(b, b + 2, 5) == b && memcmp(a, b, 20) == 0);
	strcpy(a, "0123456789");
	strcpy(b, "0123456789");
	memmove(a, a, 5);
	ft_memmove(b, b, 5);
	T(memcmp(a, b, 20) == 0);
	strcpy(a, "0123456789");
	strcpy(b, "0123456789");
	memmove(a + 1, a, 0);
	ft_memmove(b + 1, b, 0);
	T(memcmp(a, b, 20) == 0);
	memset(a, 'X', 20);
	memset(b, 'X', 20);
	memmove(a, "hello", 6);
	ft_memmove(b, "hello", 6);
	T(memcmp(a, b, 20) == 0);
	END();
}

static void	memchr_test(void)
{
	const char	s[] = "Bon\0jour 42";

	START("ft_memchr");
	T(memchr(s, 'B', 11) == ft_memchr(s, 'B', 11));
	T(memchr(s, 'o', 11) == ft_memchr(s, 'o', 11));
	T(memchr(s, 'u', 11) == ft_memchr(s, 'u', 11));
	T(memchr(s, 'u', 5) == ft_memchr(s, 'u', 5));
	T(memchr(s, '\0', 11) == ft_memchr(s, '\0', 11));
	T(memchr(s, 'B', 0) == ft_memchr(s, 'B', 0));
	T(memchr(s, 'B' + 256, 11) == ft_memchr(s, 'B' + 256, 11));
	T(memchr(s, 'z', 11) == ft_memchr(s, 'z', 11));
	T(memchr(s, '2', 11) == ft_memchr(s, '2', 11));
	END();
}

static void	memcmp_test(void)
{
	START("ft_memcmp");
	T(sgn(memcmp("abc", "abc", 3)) == sgn(ft_memcmp("abc", "abc", 3)));
	T(sgn(memcmp("abc", "abd", 3)) == sgn(ft_memcmp("abc", "abd", 3)));
	T(sgn(memcmp("abd", "abc", 3)) == sgn(ft_memcmp("abd", "abc", 3)));
	T(sgn(memcmp("abc", "abd", 2)) == sgn(ft_memcmp("abc", "abd", 2)));
	T(sgn(memcmp("abc", "xyz", 0)) == sgn(ft_memcmp("abc", "xyz", 0)));
	T(sgn(memcmp("a\0b", "a\0c", 3)) == sgn(ft_memcmp("a\0b", "a\0c", 3)));
	T(sgn(memcmp("\200", "\0", 1)) == sgn(ft_memcmp("\200", "\0", 1)));
	T(sgn(memcmp("\0", "\200", 1)) == sgn(ft_memcmp("\0", "\200", 1)));
	END();
}

static void	calloc_test(void)
{
	int		*p;
	char	*q;
	size_t	i;
	int		ok;

	START("ft_calloc");
	p = ft_calloc(10, sizeof(int));
	ok = (p != NULL);
	i = 0;
	while (ok && i < 10)
		ok = (p[i++] == 0);
	T(ok);
	free(p);
	q = ft_calloc(1, 1);
	T(q != NULL && q[0] == 0);
	free(q);
	q = ft_calloc(100, 1);
	ok = (q != NULL);
	i = 0;
	while (ok && i < 100)
		ok = (q[i++] == 0);
	T(ok);
	free(q);
	T(ft_calloc(SIZE_MAX, 2) == NULL);
	q = ft_calloc(0, 0);
	T(1);
	free(q);
	END();
}

/* ========================================================================== */
/*                       FONCTIONS ADDITIONNELLES (partie 2)                  */
/* ========================================================================== */

static void	substr_test(void)
{
	START("ft_substr");
	T(streq_free(ft_substr("hello", 1, 3), "ell"));
	T(streq_free(ft_substr("hello", 0, 5), "hello"));
	T(streq_free(ft_substr("hello", 0, 100), "hello"));
	T(streq_free(ft_substr("hello", 4, 100), "o"));
	T(streq_free(ft_substr("hello", 5, 3), ""));
	T(streq_free(ft_substr("hello", 10, 3), ""));
	T(streq_free(ft_substr("hello", 2, 0), ""));
	T(streq_free(ft_substr("", 0, 5), ""));
	END();
}

static void	strjoin_test(void)
{
	START("ft_strjoin");
	T(streq_free(ft_strjoin("foo", "bar"), "foobar"));
	T(streq_free(ft_strjoin("", ""), ""));
	T(streq_free(ft_strjoin("a", ""), "a"));
	T(streq_free(ft_strjoin("", "b"), "b"));
	T(ft_strjoin(NULL, "b") == NULL);
	T(ft_strjoin("abc", NULL) == NULL);
	T(ft_strjoin(NULL, NULL) == NULL);
	T(streq_free(ft_strjoin("Hello ", "42"), "Hello 42"));
	END();
}

static void	strtrim_test(void)
{
	START("ft_strtrim");
	T(streq_free(ft_strtrim("  abc  ", " "), "abc"));
	T(streq_free(ft_strtrim("xxabcxx", "x"), "abc"));
	T(streq_free(ft_strtrim("abc", ""), "abc"));
	T(streq_free(ft_strtrim("abc", "xyz"), "abc"));
	T(streq_free(ft_strtrim("   ", " "), ""));
	T(streq_free(ft_strtrim("", "x"), ""));
	T(streq_free(ft_strtrim("\t abc \n", " \t\n"), "abc"));
	T(streq_free(ft_strtrim("xyabcyx", "xy"), "abc"));
	T(streq_free(ft_strtrim("abcabc", "abc"), ""));
	T(streq_free(ft_strtrim("  a b  ", " "), "a b"));
	END();
}

/* verifie qu'un tableau renvoye par ft_split correspond a l'attendu */
static int	split_eq(char **res, const char **expected)
{
	size_t	i;
	int		ok;

	if (!res)
		return (0);
	ok = 1;
	i = 0;
	while (expected[i])
	{
		if (!res[i] || strcmp(res[i], expected[i]) != 0)
		{
			ok = 0;
			break ;
		}
		i++;
	}
	if (ok && res[i] != NULL)
		ok = 0;
	i = 0;
	while (res[i] && i < 100)
		free(res[i++]);
	free(res);
	return (ok);
}

static void	split_test(void)
{
	const char	*e1[] = {"hello", "world", "42", NULL};
	const char	*e2[] = {"hello", NULL};
	const char	*e3[] = {NULL};
	const char	*e4[] = {"a", "b", "c", NULL};
	const char	*e5[] = {"no_sep_here", NULL};

	START("ft_split");
	T(split_eq(ft_split("hello world 42", ' '), e1));
	T(split_eq(ft_split("  hello   world  42  ", ' '), e1));
	T(split_eq(ft_split("hello", ' '), e2));
	T(split_eq(ft_split("   hello   ", ' '), e2));
	T(split_eq(ft_split("", ' '), e3));
	T(split_eq(ft_split("     ", ' '), e3));
	T(split_eq(ft_split("a,b,c", ','), e4));
	T(split_eq(ft_split(",,a,,b,,c,,", ','), e4));
	T(split_eq(ft_split("no_sep_here", ' '), e5));
	T(split_eq(ft_split("hello world 42", '\0'), (const char *[]){"hello world 42", NULL}));
	END();
}

static void	itoa_test(void)
{
	START("ft_itoa");
	T(streq_free(ft_itoa(0), "0"));
	T(streq_free(ft_itoa(1), "1"));
	T(streq_free(ft_itoa(-1), "-1"));
	T(streq_free(ft_itoa(9), "9"));
	T(streq_free(ft_itoa(10), "10"));
	T(streq_free(ft_itoa(42), "42"));
	T(streq_free(ft_itoa(-42), "-42"));
	T(streq_free(ft_itoa(1000), "1000"));
	T(streq_free(ft_itoa(INT_MAX), "2147483647"));
	T(streq_free(ft_itoa(INT_MIN), "-2147483648"));
	END();
}

static char	f_mapi(unsigned int i, char c)
{
	if (i % 2 == 0)
		return (toupper((unsigned char)c));
	return (c);
}

static void	f_iteri(unsigned int i, char *c)
{
	if (i % 2 == 0)
		*c = toupper((unsigned char)*c);
}

static void	strmapi_test(void)
{
	START("ft_strmapi");
	T(streq_free(ft_strmapi("abcdef", f_mapi), "AbCdEf"));
	T(streq_free(ft_strmapi("a", f_mapi), "A"));
	T(streq_free(ft_strmapi("", f_mapi), ""));
	T(streq_free(ft_strmapi("hello 42", f_mapi), "HeLlO 42"));
	END();
}

static void	striteri_test(void)
{
	char	a[] = "abcdef";
	char	b[] = "";
	char	c[] = "hello 42";

	START("ft_striteri");
	ft_striteri(a, f_iteri);
	T(strcmp(a, "AbCdEf") == 0);
	ft_striteri(b, f_iteri);
	T(strcmp(b, "") == 0);
	ft_striteri(c, f_iteri);
	T(strcmp(c, "HeLlO 42") == 0);
	END();
}

/* ===== fonctions *_fd : on redirige vers un pipe puis on relit ===== */

static void	pipe_read(int p[2], char *buf, size_t n)
{
	ssize_t	r;

	close(p[1]);
	r = read(p[0], buf, n - 1);
	if (r < 0)
		r = 0;
	buf[r] = '\0';
	close(p[0]);
}

static void	putchar_fd_test(void)
{
	int		p[2];
	char	buf[64];

	START("ft_putchar_fd");
	if (pipe(p) == 0)
	{
		ft_putchar_fd('a', p[1]);
		ft_putchar_fd('\n', p[1]);
		ft_putchar_fd('4', p[1]);
		pipe_read(p, buf, sizeof(buf));
		T(strcmp(buf, "a\n4") == 0);
	}
	END();
}

static void	putstr_fd_test(void)
{
	int		p[2];
	char	buf[64];

	START("ft_putstr_fd");
	if (pipe(p) == 0)
	{
		ft_putstr_fd("Hello 42", p[1]);
		ft_putstr_fd("", p[1]);
		ft_putstr_fd("!\n", p[1]);
		pipe_read(p, buf, sizeof(buf));
		T(strcmp(buf, "Hello 42!\n") == 0);
	}
	END();
}

static void	putendl_fd_test(void)
{
	int		p[2];
	char	buf[64];

	START("ft_putendl_fd");
	if (pipe(p) == 0)
	{
		ft_putendl_fd("Hello", p[1]);
		ft_putendl_fd("", p[1]);
		ft_putendl_fd("42", p[1]);
		pipe_read(p, buf, sizeof(buf));
		T(strcmp(buf, "Hello\n\n42\n") == 0);
	}
	END();
}

static void	t_putnbr(int n, const char *expected)
{
	int		p[2];
	char	buf[64];

	if (pipe(p) == 0)
	{
		ft_putnbr_fd(n, p[1]);
		pipe_read(p, buf, sizeof(buf));
		T(strcmp(buf, expected) == 0);
	}
}

static void	putnbr_fd_test(void)
{
	START("ft_putnbr_fd");
	t_putnbr(0, "0");
	t_putnbr(5, "5");
	t_putnbr(-5, "-5");
	t_putnbr(42, "42");
	t_putnbr(-42, "-42");
	t_putnbr(1000, "1000");
	t_putnbr(INT_MAX, "2147483647");
	t_putnbr(INT_MIN, "-2147483648");
	END();
}

/* ========================================================================== */
/*                                LISTES CHAINEES                             */
/* ========================================================================== */

static int	g_del_count = 0;

static void	del_free(void *c) { free(c); g_del_count++; }
static void	del_noop(void *c) { (void)c; g_del_count++; }

static void	*f_dup_upper(void *c)
{
	char	*s;
	size_t	i;

	s = strdup((char *)c);
	if (!s)
		return (NULL);
	i = 0;
	while (s[i])
	{
		s[i] = toupper((unsigned char)s[i]);
		i++;
	}
	return (s);
}

static void	f_iter_upper(void *c)
{
	char	*s;

	s = (char *)c;
	while (*s)
	{
		*s = toupper((unsigned char)*s);
		s++;
	}
}

static void	lstnew_test(void)
{
	t_list	*l;
	int		x;

	START("ft_lstnew");
	x = 42;
	l = ft_lstnew(&x);
	T(l != NULL && l->content == &x && l->next == NULL);
	free(l);
	l = ft_lstnew(NULL);
	T(l != NULL && l->content == NULL && l->next == NULL);
	free(l);
	END();
}

static void	lstadd_front_test(void)
{
	t_list	*lst;
	t_list	*a;
	t_list	*b;

	START("ft_lstadd_front");
	lst = NULL;
	a = ft_lstnew("A");
	b = ft_lstnew("B");
	ft_lstadd_front(&lst, a);
	T(lst == a && a->next == NULL);
	ft_lstadd_front(&lst, b);
	T(lst == b && b->next == a && a->next == NULL);
	free(a);
	free(b);
	END();
}

static void	lstsize_test(void)
{
	t_list	*lst;
	t_list	*a;
	t_list	*b;
	t_list	*c;

	START("ft_lstsize");
	lst = NULL;
	T(ft_lstsize(lst) == 0);
	a = ft_lstnew("A");
	b = ft_lstnew("B");
	c = ft_lstnew("C");
	lst = a;
	T(ft_lstsize(lst) == 1);
	a->next = b;
	T(ft_lstsize(lst) == 2);
	b->next = c;
	T(ft_lstsize(lst) == 3);
	free(a);
	free(b);
	free(c);
	END();
}

static void	lstlast_test(void)
{
	t_list	*a;
	t_list	*b;
	t_list	*c;

	START("ft_lstlast");
	T(ft_lstlast(NULL) == NULL);
	a = ft_lstnew("A");
	b = ft_lstnew("B");
	c = ft_lstnew("C");
	T(ft_lstlast(a) == a);
	a->next = b;
	T(ft_lstlast(a) == b);
	b->next = c;
	T(ft_lstlast(a) == c);
	T(ft_lstlast(b) == c);
	free(a);
	free(b);
	free(c);
	END();
}

static void	lstadd_back_test(void)
{
	t_list	*lst;
	t_list	*a;
	t_list	*b;
	t_list	*c;

	START("ft_lstadd_back");
	lst = NULL;
	a = ft_lstnew("A");
	b = ft_lstnew("B");
	c = ft_lstnew("C");
	ft_lstadd_back(&lst, a);
	T(lst == a && a->next == NULL);
	ft_lstadd_back(&lst, b);
	T(lst == a && a->next == b && b->next == NULL);
	ft_lstadd_back(&lst, c);
	T(lst == a && b->next == c && c->next == NULL);
	T(ft_lstsize(lst) == 3);
	free(a);
	free(b);
	free(c);
	END();
}

static void	lstdelone_test(void)
{
	t_list	*l;

	START("ft_lstdelone");
	g_del_count = 0;
	l = ft_lstnew(strdup("to delete"));
	ft_lstdelone(l, del_free);
	T(g_del_count == 1);
	END();
}

static void	lstclear_test(void)
{
	t_list	*lst;

	START("ft_lstclear");
	g_del_count = 0;
	lst = NULL;
	ft_lstadd_back(&lst, ft_lstnew(strdup("1")));
	ft_lstadd_back(&lst, ft_lstnew(strdup("2")));
	ft_lstadd_back(&lst, ft_lstnew(strdup("3")));
	ft_lstclear(&lst, del_free);
	T(lst == NULL);
	T(g_del_count == 3);
	ft_lstclear(&lst, del_free);
	T(lst == NULL && g_del_count == 3);
	END();
}

static void	lstiter_test(void)
{
	t_list	*lst;

	START("ft_lstiter");
	lst = NULL;
	ft_lstadd_back(&lst, ft_lstnew(strdup("abc")));
	ft_lstadd_back(&lst, ft_lstnew(strdup("def")));
	ft_lstiter(lst, f_iter_upper);
	T(strcmp((char *)lst->content, "ABC") == 0);
	T(strcmp((char *)lst->next->content, "DEF") == 0);
	ft_lstiter(NULL, f_iter_upper);
	T(1);
	ft_lstclear(&lst, del_free);
	END();
}

static void	lstmap_test(void)
{
	t_list	*lst;
	t_list	*m;

	START("ft_lstmap");
	lst = NULL;
	ft_lstadd_back(&lst, ft_lstnew(strdup("abc")));
	ft_lstadd_back(&lst, ft_lstnew(strdup("def")));
	ft_lstadd_back(&lst, ft_lstnew(strdup("ghi")));
	m = ft_lstmap(lst, f_dup_upper, del_free);
	T(m != NULL && ft_lstsize(m) == 3);
	T(m != NULL && strcmp((char *)m->content, "ABC") == 0);
	T(m != NULL && strcmp((char *)m->next->content, "DEF") == 0);
	T(m != NULL && strcmp((char *)m->next->next->content, "GHI") == 0);
	T(strcmp((char *)lst->content, "abc") == 0);
	T(m != lst);
	ft_lstclear(&m, del_free);
	ft_lstclear(&lst, del_free);
	T(ft_lstmap(NULL, f_dup_upper, del_free) == NULL);
	(void)del_noop;
	END();
}

/* ========================================================================== */
/*                                    MAIN                                    */
/* ========================================================================== */

int	main(void)
{
	printf("\n===== PARTIE 1 : LIBC =====\n");
	isalpha_test();
	isdigit_test();
	isalnum_test();
	isascii_test();
	isprint_test();
	tolower_test();
	toupper_test();
	strlen_test();
	strlcpy_test();
	strlcat_test();
	strncmp_test();
	strdup_test();
	strchr_test();
	strrchr_test();
	strnstr_test();
	atoi_test();
	bzero_test();
	memset_test();
	memcpy_test();
	memmove_test();
	memchr_test();
	memcmp_test();
	calloc_test();
	printf("\n===== PARTIE 2 : FONCTIONS ADDITIONNELLES =====\n");
	substr_test();
	strjoin_test();
	strtrim_test();
	split_test();
	itoa_test();
	strmapi_test();
	striteri_test();
	putchar_fd_test();
	putstr_fd_test();
	putendl_fd_test();
	putnbr_fd_test();
	printf("\n===== BONUS : LISTES CHAINEES =====\n");
	lstnew_test();
	lstadd_front_test();
	lstsize_test();
	lstlast_test();
	lstadd_back_test();
	lstdelone_test();
	lstclear_test();
	lstiter_test();
	lstmap_test();
	printf("\n%s (%d test(s) en echec)\n\n",
		g_fail ? "RESULTAT : KO" : "RESULTAT : TOUT EST OK", g_fail);
	return (g_fail != 0);
}