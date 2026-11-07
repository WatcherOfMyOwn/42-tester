#include "main.h"

int	g_fail = 0;

/* ===== malloc "pilotable" : permet de faire echouer le N-ieme malloc ===== */
/* (actif seulement si g_arm = 1, et seulement pour les malloc appeles depuis  */
/*  libft.a et ce testeur : link avec -Wl,--wrap=malloc)                       */

static int	g_arm = 0;
static int	g_calls = 0;
static int	g_fail_at = 0;

void	*__real_malloc(size_t n);
void	*__wrap_malloc(size_t n)
{
	if (g_arm)
	{
		g_calls++;
		if (g_calls == g_fail_at)
			return (NULL);
	}
	return (__real_malloc(n));
}

/* copie EXACTE sur le tas : sous ASan, toute lecture/ecriture hors zone plante */
static void	*hdup(const void *src, size_t n)
{
	void	*p;

	p = malloc(n ? n : 1);
	if (p && n)
		memcpy(p, src, n);
	return (p);
}

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

/* compare libc et ft sur -1..255 ; ft doit renvoyer EXACTEMENT 1 ou 0 (sujet) */
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
		if (ft_##fn(i) != (fn(i) != 0) && bad == -2) \
			bad = i; \
		i++; \
	} \
	if (bad == -2) \
		printf(GREEN "OK (-1..255)" RESET); \
	else \
		{ printf(RED "KO (premier ecart : c = %d)" RESET, bad); g_fail++; } \
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
		printf(GREEN "OK (-1..255)" RESET); \
	else \
		{ printf(RED "KO (premier ecart : c = %d)" RESET, bad); g_fail++; } \
	END(); \
}

CASE_TEST(tolower)
CASE_TEST(toupper)

/* ========================================================================== */
/*                                 STRING UTILS                               */
/* ========================================================================== */

static void	t_strlen(const char *src)
{
	char	*x;

	x = hdup(src, strlen(src) + 1);
	T(strlen(x) == ft_strlen(x));
	free(x);
}

static void	strlen_test(void)
{
	START("ft_strlen");
	t_strlen("");
	t_strlen("a");
	t_strlen("Bonjour");
	t_strlen("a\0b");
	t_strlen("   \t\n  ");
	t_strlen("0123456789012345678901234567890123456789");
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

/* ===== comparaisons libc / ft sur des copies EXACTES allouees sur le tas ===== */

static void	t_strncmp(const char *a, const char *b, size_t n)
{
	char	*x;
	char	*y;

	x = hdup(a, strlen(a) + 1);
	y = hdup(b, strlen(b) + 1);
	T(sgn(strncmp(x, y, n)) == sgn(ft_strncmp(x, y, n)));
	free(x);
	free(y);
}

static void	t_strchr(const char *s, int c)
{
	char	*x;

	x = hdup(s, strlen(s) + 1);
	T(strchr(x, c) == ft_strchr(x, c));
	free(x);
}

static void	t_strrchr(const char *s, int c)
{
	char	*x;

	x = hdup(s, strlen(s) + 1);
	T(strrchr(x, c) == ft_strrchr(x, c));
	free(x);
}

static void	t_strnstr(const char *big, const char *little, size_t len)
{
	char	*x;
	char	*y;

	x = hdup(big, strlen(big) + 1);
	y = hdup(little, strlen(little) + 1);
	T(ref_strnstr(x, y, len) == ft_strnstr(x, y, len));
	free(x);
	free(y);
}

static void	t_memchr(const void *s, int c, size_t n)
{
	void	*x;

	x = hdup(s, n);
	T(memchr(x, c, n) == ft_memchr(x, c, n));
	free(x);
}

static void	t_memcmp(const void *a, const void *b, size_t n)
{
	void	*x;
	void	*y;

	x = hdup(a, n);
	y = hdup(b, n);
	T(sgn(memcmp(x, y, n)) == sgn(ft_memcmp(x, y, n)));
	free(x);
	free(y);
}

static void	strncmp_test(void)
{
	START("ft_strncmp");
	t_strncmp("abc", "abc", 3);
	t_strncmp("abc", "abd", 3);
	t_strncmp("abd", "abc", 3);
	t_strncmp("abc", "abd", 2);
	t_strncmp("abc", "abd", 0);
	t_strncmp("abc", "abcd", 10);
	t_strncmp("abcd", "abc", 10);
	t_strncmp("", "", 5);
	t_strncmp("", "a", 5);
	t_strncmp("test\200", "test\0", 6);
	t_strncmp("\200", "\0", 1);
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
	t_strchr(s, 'H');
	t_strchr(s, 'l');
	t_strchr(s, 'l' + 256);
	t_strchr(s, 'o');
	t_strchr(s, 'z');
	t_strchr(s, '\0');
	t_strchr("", '\0');
	t_strchr("", 'a');
	END();
}

static void	strrchr_test(void)
{
	const char	*s = "Hello, 42 school";

	START("ft_strrchr");
	t_strrchr(s, 'H');
	t_strrchr(s, 'l');
	t_strrchr(s, 'o');
	t_strrchr(s, 'z');
	t_strrchr(s, '\0');
	t_strrchr("", '\0');
	t_strrchr("", 'a');
	END();
}

static void	strnstr_test(void)
{
	const char	*big = "Foo Bar Baz";

	START("ft_strnstr");
	t_strnstr(big, "Bar", 11);
	t_strnstr(big, "Bar", 7);
	t_strnstr(big, "Bar", 6);
	t_strnstr(big, "Baz", 11);
	t_strnstr(big, "Baz", 10);
	t_strnstr(big, "Foo", 0);
	t_strnstr(big, "", 0);
	t_strnstr(big, "", 5);
	t_strnstr(big, "xyz", 11);
	t_strnstr("", "a", 5);
	t_strnstr("", "", 5);
	t_strnstr(big, "Foo Bar Baz Qux", 20);
	t_strnstr("aaabc", "abc", 5);
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
	t_memchr(s, 'B', 11);
	t_memchr(s, 'o', 11);
	t_memchr(s, 'u', 11);
	t_memchr(s, 'u', 5);
	t_memchr(s, '\0', 11);
	t_memchr(s, 'B', 0);
	t_memchr(s, 'B' + 256, 11);
	t_memchr(s, 'z', 11);
	t_memchr(s, '2', 11);
	END();
}

static void	memcmp_test(void)
{
	START("ft_memcmp");
	t_memcmp("abc", "abc", 3);
	t_memcmp("abc", "abd", 3);
	t_memcmp("abd", "abc", 3);
	t_memcmp("abc", "abd", 2);
	t_memcmp("abc", "xyz", 0);
	t_memcmp("a\0b", "a\0c", 3);
	t_memcmp("\200", "\0", 1);
	t_memcmp("\0", "\200", 1);
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
	T(q != NULL);
	free(q);
	q = ft_calloc(0, 4);
	T(q != NULL);
	free(q);
	q = ft_calloc(4, 0);
	T(q != NULL);
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
	T(streq_free(ft_substr("hello", 1, SIZE_MAX), "ello"));
	T(streq_free(ft_substr("hello", UINT_MAX, 3), ""));
	T(streq_free(ft_substr("hello", 0, SIZE_MAX), "hello"));
	END();
}

static void	strjoin_test(void)
{
	START("ft_strjoin");
	T(streq_free(ft_strjoin("foo", "bar"), "foobar"));
	T(streq_free(ft_strjoin("", ""), ""));
	T(streq_free(ft_strjoin("a", ""), "a"));
	T(streq_free(ft_strjoin("", "b"), "b"));
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
	(void)del_noop;
	END();
}

/* ========================================================================== */
/*                  ECHECS DE MALLOC (retour NULL + pas de fuite)             */
/* ========================================================================== */

static void	cl_free(void *p)
{
	free(p);
}

static void	cl_tab(void *p)
{
	char	**t;
	size_t	i;

	t = (char **)p;
	if (!t)
		return ;
	i = 0;
	while (t[i])
		free(t[i++]);
	free(t);
}

/* Compte d'abord les malloc d'un appel reussi (n), puis refait l'appel en    */
/* faisant echouer le 1er, le 2e, ... le n-ieme malloc : on doit toujours     */
/* obtenir NULL. Les fuites eventuelles se voient avec `make asan` ou `make vg`*/
#define MFAIL(label, expr, cleanup) \
do { \
	int		n_; \
	int		k_; \
	void	*r_; \
	START(label); \
	g_fail_at = 0; \
	g_calls = 0; \
	g_arm = 1; \
	cleanup((void *)(expr)); \
	g_arm = 0; \
	n_ = g_calls; \
	T(n_ > 0); \
	k_ = 1; \
	while (k_ <= n_) \
	{ \
		g_fail_at = k_; \
		g_calls = 0; \
		g_arm = 1; \
		r_ = (void *)(expr); \
		g_arm = 0; \
		T(r_ == NULL); \
		cleanup(r_); \
		k_++; \
	} \
	g_fail_at = 0; \
	END(); \
} while (0)

static int	g_f_calls = 0;

static void	*f_dup_count(void *c)
{
	g_f_calls++;
	return (strdup((char *)c));
}

static void	lstmap_fail_test(void)
{
	t_list	*lst;
	t_list	*m;
	int		n;
	int		k;

	START("ft_lstmap (KO)");
	lst = NULL;
	ft_lstadd_back(&lst, ft_lstnew(strdup("abc")));
	ft_lstadd_back(&lst, ft_lstnew(strdup("def")));
	ft_lstadd_back(&lst, ft_lstnew(strdup("ghi")));
	g_fail_at = 0;
	g_calls = 0;
	g_arm = 1;
	m = ft_lstmap(lst, f_dup_count, del_free);
	g_arm = 0;
	n = g_calls;
	ft_lstclear(&m, del_free);
	k = 1;
	while (k <= n)
	{
		g_f_calls = 0;
		g_del_count = 0;
		g_fail_at = k;
		g_calls = 0;
		g_arm = 1;
		m = ft_lstmap(lst, f_dup_count, del_free);
		g_arm = 0;
		T(m == NULL);
		/* chaque contenu cree par f doit avoir ete detruit par del */
		T(g_f_calls == g_del_count);
		k++;
	}
	g_fail_at = 0;
	ft_lstclear(&lst, del_free);
	END();
}

static void	malloc_fail_test(void)
{
	MFAIL("ft_strdup (KO)", ft_strdup("hello"), cl_free);
	MFAIL("ft_substr (KO)", ft_substr("hello", 1, 3), cl_free);
	MFAIL("ft_strjoin (KO)", ft_strjoin("foo", "bar"), cl_free);
	MFAIL("ft_strtrim (KO)", ft_strtrim("  abc  ", " "), cl_free);
	MFAIL("ft_split (KO)", ft_split("a bc d", ' '), cl_tab);
	MFAIL("ft_itoa (KO)", ft_itoa(-42), cl_free);
	MFAIL("ft_strmapi (KO)", ft_strmapi("abc", f_mapi), cl_free);
	MFAIL("ft_calloc (KO)", ft_calloc(4, 4), cl_free);
	MFAIL("ft_lstnew (KO)", ft_lstnew("x"), cl_free);
	lstmap_fail_test();
}

/* ========================================================================== */
/*          CAS LIMITES / NULL : chaque cas tourne dans un processus fils     */
/* ========================================================================== */

/* 1 si fn se termine normalement, 0 si elle plante (segfault, ASan...).      */
/* Avec ASan, un crash sort avec un code != 0 au lieu d'un signal.            */
static int  survives(void (*fn)(void))
{
    pid_t   pid;
    int     status;

    fflush(NULL);
    pid = fork();
    if (pid == 0)
    {
        dup2(open("/dev/null", O_WRONLY), 2);
        fn();
        _exit(0);
    }
    waitpid(pid, &status, 0);
    return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/* cas VALIDES (liste vide, taille extreme, fd invalide...) : ne doivent pas planter */
static void must_survive(const char *name, void (*fn)(void))
{
    int ok;

    ok = survives(fn);
    printf("  %-34s %s\n", name, ok ? GREEN "OK" RESET : RED "KO (CRASH)" RESET);
    if (!ok)
        g_fail++;
}

/* cas NON DEFINIS : on compare avec la libc quand elle existe. Si la libc    */
/* plante aussi, c'est un UB : rien n'est exige. Ce n'est jamais compte en KO */
/* sauf si la libc survit et pas toi.                                         */
static void info(const char *name, void (*libc)(void), void (*ft)(void))
{
    int l;
    int f;

    f = survives(ft);
    if (!libc)
    {
        printf("  %-34s ft: %-6s " YELLOW " (undefined by the subject) " RESET "\n", name, f ? GREEN "OK" RESET : ORANGE "CRASH" RESET);
        return ;
    }
    l = survives(libc);
    printf("  %-34s libc: %-6s ft: %-6s %s\n", name, l ? GREEN "OK" RESET : ORANGE "CRASH" RESET,
        f ? GREEN "OK" RESET : ORANGE "CRASH" RESET, !l ? YELLOW "(UB : nothing required)" RESET : f ? "" : ORANGE "<- COULD BE FIX" RESET);
}

static void del_nop(void *c) { (void)c; }
static void f_nop(void *c) { (void)c; }
static void *f_id(void *c) { return (c); }
static char f_mapi_id(unsigned int i, char c) { (void)i; return (c); }
static void f_iteri_nop(unsigned int i, char *c) { (void)i; (void)c; }

static void free_tab(char **tab)
{
    int i = 0;
    if (!tab)
        return;
    while (tab[i])
        free(tab[i++]);
    free(tab);
}

/* --- cas valides --- */
static void v_lstsize_null(void) { ft_lstsize(NULL); }
static void v_lstlast_null(void) { ft_lstlast(NULL); }
static void v_lstiter_null(void) { ft_lstiter(NULL, f_nop); }
static void v_lstmap_null(void) { ft_lstmap(NULL, f_id, del_nop); }
static void v_lstadd_back_empty(void)
{
    t_list  *l;

    l = NULL;
    ft_lstadd_back(&l, ft_lstnew(NULL));
	ft_lstclear(&l, del_nop);
}
static void v_lstadd_front_empty(void)
{
    t_list  *l;

    l = NULL;
    ft_lstadd_front(&l, ft_lstnew(NULL));
	ft_lstclear(&l, del_nop);
}
static void v_lstclear_empty(void)
{
    t_list  *l;

    l = NULL;
    ft_lstclear(&l, del_nop);
}
static void v_putchar_badfd(void) { ft_putchar_fd('a', -1); }
static void v_putstr_badfd(void) { ft_putstr_fd("abc", -1); }
static void v_putendl_badfd(void) { ft_putendl_fd("abc", -1); }
static void v_putnbr_badfd(void) { ft_putnbr_fd(INT_MIN, -1); }
static void v_substr_huge(void) { free(ft_substr("hello", 1, SIZE_MAX)); }
static void v_substr_hugestart(void) { free(ft_substr("hello", UINT_MAX, 3)); }
static void v_calloc_zero_0(void) { free(ft_calloc(0, 0)); }
static void v_calloc_zero_1(void) { free(ft_calloc(0, 42)); }
static void v_calloc_zero_2(void) { free(ft_calloc(42, 0)); }
static void v_itoa_intmin(void) { free(ft_itoa(INT_MIN)); }
static void v_split_empty(void) { free_tab(ft_split("", ' ')); }
static void v_split_spaces(void) { free_tab(ft_split("   ", ' ')); }
static void v_split_consec(void) { free_tab(ft_split("a,,b", ',')); }

/* --- cas non definis --- */
static void u_strdup_null(void) { free(ft_strdup(NULL)); }
static void u_strlen_null(void) { volatile size_t r = ft_strlen(NULL); (void)r; }
static void u_atoi_null(void) { volatile int r = ft_atoi(NULL); (void)r; }
static void l_memset_null_0(void)   { void *p = NULL; size_t n = 0; volatile void *r = memset(p, 'a', n); (void)r; }
static void u_memset_null_0(void)   { void *p = NULL; size_t n = 0; volatile void *r = ft_memset(p, 'a', n); (void)r; }
static void l_memset_null_n(void)   { void *p = NULL; size_t n = 5; volatile void *r = memset(p, 'a', n); (void)r; }
static void u_memset_null_n(void)   { void *p = NULL; size_t n = 5; volatile void *r = ft_memset(p, 'a', n); (void)r; }
static void l_memset_null_max(void) { void *p = NULL; size_t n = SIZE_MAX; volatile void *r = memset(p, 'a', n >> 1); (void)r; }
static void u_memset_null_max(void) { void *p = NULL; size_t n = SIZE_MAX; volatile void *r = ft_memset(p, 'a', n >> 1); (void)r; }
static void l_bzero_null_0(void)   { void *p = NULL; size_t n = 0; bzero(p, n); }
static void u_bzero_null_0(void)   { void *p = NULL; size_t n = 0; ft_bzero(p, n); }
static void l_bzero_null_n(void)   { void *p = NULL; size_t n = 5; bzero(p, n); }
static void u_bzero_null_n(void)   { void *p = NULL; size_t n = 5; ft_bzero(p, n); }
static void l_bzero_null_max(void) { void *p = NULL; size_t n = SIZE_MAX; bzero(p, n >> 1); }
static void u_bzero_null_max(void) { void *p = NULL; size_t n = SIZE_MAX; ft_bzero(p, n >> 1); }
static void l_memcpy_null_0(void)   { void *p = NULL; void *q = NULL; size_t n = 0; volatile void *r = memcpy(p, q, n); (void)r; }
static void u_memcpy_null_0(void)   { void *p = NULL; void *q = NULL; size_t n = 0; volatile void *r = ft_memcpy(p, q, n); (void)r; }
static void l_memcpy_null_n(void)   { void *p = NULL; void *q = NULL; size_t n = 5; volatile void *r = memcpy(p, q, n); (void)r; }
static void u_memcpy_null_n(void)   { void *p = NULL; void *q = NULL; size_t n = 5; volatile void *r = ft_memcpy(p, q, n); (void)r; }
static void l_memcpy_null_max(void) { void *p = NULL; void *q = NULL; size_t n = SIZE_MAX; volatile void *r = memcpy(p, q, n >> 1); (void)r; }
static void u_memcpy_null_max(void) { void *p = NULL; void *q = NULL; size_t n = SIZE_MAX; volatile void *r = ft_memcpy(p, q, n >> 1); (void)r; }
static void l_memmove_null_0(void)   { void *p = NULL; size_t n = 0; volatile void *r = memmove(p, p, n); (void)r; }
static void u_memmove_null_0(void)   { void *p = NULL; size_t n = 0; volatile void *r = ft_memmove(p, p, n); (void)r; }
static void l_memmove_null_n(void)   { void *p = NULL; size_t n = 5; volatile void *r = memmove(p, p, n); (void)r; }
static void u_memmove_null_n(void)   { void *p = NULL; size_t n = 5; volatile void *r = ft_memmove(p, p, n); (void)r; }
static void l_memmove_null_max(void) { void *p = NULL; size_t n = SIZE_MAX; volatile void *r = memmove(p, p, n >> 1); (void)r; }
static void u_memmove_null_max(void) { void *p = NULL; size_t n = SIZE_MAX; volatile void *r = ft_memmove(p, p, n >> 1); (void)r; }
static void l_memchr_null_0(void)   { void *p = NULL; size_t n = 0; volatile void *r = memchr(p, 'a', n); (void)r; }
static void u_memchr_null_0(void)   { const void *p = NULL; size_t n = 0; volatile void *r = ft_memchr(p, 'a', n); (void)r; }
static void l_memchr_null_n(void)   { void *p = NULL; size_t n = 5; volatile void *r = memchr(p, 'a', n); (void)r; }
static void u_memchr_null_n(void)   { const void *p = NULL; size_t n = 5; volatile void *r = ft_memchr(p, 'a', n); (void)r; }
static void l_memchr_null_max(void) { void *p = NULL; size_t n = SIZE_MAX; volatile void *r = memchr(p, 'a', n); (void)r; }
static void u_memchr_null_max(void) { const void *p = NULL; size_t n = SIZE_MAX; volatile void *r = ft_memchr(p, 'a', n); (void)r; }
static void l_memcmp_null_0(void)   { void *p = NULL; size_t n = 0; volatile int r = memcmp(p, p, n); (void)r; }
static void u_memcmp_null_0(void)   { const void *p = NULL; size_t n = 0; volatile int r = ft_memcmp(p, p, n); (void)r; }
static void l_memcmp_null_n(void)   { void *p = NULL; size_t n = 5; volatile int r = memcmp(p, p, n); (void)r; }
static void u_memcmp_null_n(void)   { const void *p = NULL; size_t n = 5; volatile int r = ft_memcmp(p, p, n); (void)r; }
static void l_memcmp_null_max(void) { void *p = NULL; size_t n = SIZE_MAX; volatile int r = memcmp(p, p, n); (void)r; }
static void u_memcmp_null_max(void) { const void *p = NULL; size_t n = SIZE_MAX; volatile int r = ft_memcmp(p, p, n); (void)r; }
static void l_strdup_null(void) { const char *volatile p = NULL; free(strdup(p)); }
static void l_strlen_null(void) { const char *volatile p = NULL; volatile size_t r = strlen(p); (void)r; }
static void l_atoi_null(void) { const char *volatile p = NULL; volatile int r = atoi(p); (void)r; }
static void u_substr_null(void) { free(ft_substr(NULL, 0, 3)); }
static void u_strjoin_null1(void) { free(ft_strjoin(NULL, "b")); }
static void u_strjoin_null2(void) { free(ft_strjoin("a", NULL)); }
static void u_strtrim_null1(void) { free(ft_strtrim(NULL, "x")); }
static void u_strtrim_null2(void) { free(ft_strtrim("x", NULL)); }
static void u_split_null(void) { free_tab(ft_split(NULL, ' ')); }
static void u_strmapi_null1(void) { free(ft_strmapi(NULL, f_mapi_id)); }
static void u_strmapi_null2(void) { free(ft_strmapi("abc", NULL)); }
static void u_striteri_null1(void) { ft_striteri(NULL, f_iteri_nop); }
static void u_striteri_null2(void) { char s[] = "abc"; ft_striteri(s, NULL); }
static void u_putstr_null(void) { ft_putstr_fd(NULL, 1); }
static void u_putendl_null(void) { ft_putendl_fd(NULL, 1); }
static void u_lstadd_back_null(void) { 
    t_list *node = ft_lstnew(NULL);
    ft_lstadd_back(NULL, node); 
    free(node);
}
static void u_lstadd_front_null(void) { 
    t_list *node = ft_lstnew(NULL);
    ft_lstadd_front(NULL, node); 
    free(node);
}
static void u_lstdelone_null(void) { ft_lstdelone(NULL, del_nop); }
static void u_lstclear_null(void) { ft_lstclear(NULL, del_nop); }

static void edge_cases(void)
{
    /* Calloc & Itoa */
    must_survive("ft_calloc(0, 0)", v_calloc_zero_0);
    must_survive("ft_calloc(0, 42)", v_calloc_zero_1);
    must_survive("ft_calloc(42, 0)", v_calloc_zero_2);
    must_survive("ft_itoa(INT_MIN)", v_itoa_intmin);

    /* Substr (Tailles extrêmes) */
    must_survive("ft_substr(s, 1, SIZE_MAX)", v_substr_huge);
    must_survive("ft_substr(s, UINT_MAX, 3)", v_substr_hugestart);

    /* FD Invalides */
    must_survive("ft_putchar_fd(fd = -1)", v_putchar_badfd);
    must_survive("ft_putstr_fd(\"abc\", fd = -1)", v_putstr_badfd);
    must_survive("ft_putendl_fd(\"abc\", fd = -1)", v_putendl_badfd);
    must_survive("ft_putnbr_fd(INT_MIN, fd = -1)", v_putnbr_badfd);

    /* Listes (Cas limites valides) */
    must_survive("ft_lstsize(NULL)", v_lstsize_null);
    must_survive("ft_lstlast(NULL)", v_lstlast_null);
    must_survive("ft_lstiter(NULL, f)", v_lstiter_null);
    must_survive("ft_lstmap(NULL, f, del)", v_lstmap_null);
    must_survive("ft_lstadd_back(&vide, new)", v_lstadd_back_empty);
    must_survive("ft_lstadd_front(&vide, new)", v_lstadd_front_empty);
    must_survive("ft_lstclear(&vide, del)", v_lstclear_empty);
}

static void undefined_cases(void)
{
    /* Partie 1 : Libc (Chaînes & Conversions NULL) */
	info("strdup(NULL)", l_strdup_null, u_strdup_null);
	info("strlen(NULL)", l_strlen_null, u_strlen_null);
	info("atoi(NULL)", l_atoi_null, u_atoi_null);

    /* Partie 1 : Libc (Fonctions Mémoire NULL avec n = 0) */
	info("memset(NULL, 'a', 0)", l_memset_null_0, u_memset_null_0);
	info("memset(NULL, 'a', 5)", l_memset_null_n, u_memset_null_n);
	info("memset(NULL, 'a', SIZE_MAX)", l_memset_null_max, u_memset_null_max);
	info("bzero(NULL, 0)", l_bzero_null_0, u_bzero_null_0);
	info("bzero(NULL, 5)", l_bzero_null_n, u_bzero_null_n);
	info("bzero(NULL, SIZE_MAX)", l_bzero_null_max, u_bzero_null_max);
	info("memcpy(NULL, NULL, 0)", l_memcpy_null_0, u_memcpy_null_0);
	info("memcpy(NULL, NULL, 5)", l_memcpy_null_n, u_memcpy_null_n);
	info("memcpy(NULL, NULL, SIZE_MAX)", l_memcpy_null_max, u_memcpy_null_max);
	info("memmove(NULL, NULL, 0)", l_memmove_null_0, u_memmove_null_0);
	info("memmove(NULL, NULL, 5)", l_memmove_null_n, u_memmove_null_n);
	info("memmove(NULL, NULL, SIZE_MAX)", l_memmove_null_max, u_memmove_null_max);
	info("memchr(NULL, 'a', 0)", l_memchr_null_0, u_memchr_null_0);
	info("memchr(NULL, 'a', 5)", l_memchr_null_n, u_memchr_null_n);
	info("memchr(NULL, 'a', SIZE_MAX)", l_memchr_null_max, u_memchr_null_max);
	info("memcmp(NULL, NULL, 0)", l_memcmp_null_0, u_memcmp_null_0);
	info("memcmp(NULL, NULL, 5)", l_memcmp_null_n, u_memcmp_null_n);
	info("memcmp(NULL, NULL, SIZE_MAX)", l_memcmp_null_max, u_memcmp_null_max);

	/* Partie 2 : Manipulation de chaînes NULL */
	info("ft_substr(NULL, 0, 3)", NULL, u_substr_null);
	info("ft_strjoin(NULL, \"b\")", NULL, u_strjoin_null1);
	info("ft_strjoin(\"a\", NULL)", NULL, u_strjoin_null2);
	info("ft_strtrim(NULL, \"x\")", NULL, u_strtrim_null1);
	info("ft_strtrim(\"x\", NULL)", NULL, u_strtrim_null2);
	info("ft_strmapi(NULL, f)", NULL, u_strmapi_null1);
	info("ft_strmapi(s, NULL)", NULL, u_strmapi_null2);
	info("ft_striteri(NULL, f)", NULL, u_striteri_null1);
	info("ft_striteri(s, NULL)", NULL, u_striteri_null2);

	/* Partie 2 : Split */
	info("ft_split(NULL, ' ')", NULL, u_split_null);
	info("ft_split(\"\", ' ')", NULL, v_split_empty);
	info("ft_split(\"   \", ' ')", NULL, v_split_spaces);
	info("ft_split(\"a,,b\", ',')", NULL, v_split_consec);

	/* Partie 2 : FD NULL */
	info("ft_putstr_fd(NULL, 1)", NULL, u_putstr_null);
	info("ft_putendl_fd(NULL, 1)", NULL, u_putendl_null);

	/* Partie 3 : Listes NULL */
	info("ft_lstadd_back(NULL, new)", NULL, u_lstadd_back_null);
	info("ft_lstadd_front(NULL, new)", NULL, u_lstadd_front_null);
	info("ft_lstdelone(NULL, del)", NULL, u_lstdelone_null);
	info("ft_lstclear(NULL, del)", NULL, u_lstclear_null);
}

/* ========================================================================== */
/*                                    MAIN                                    */
/* ========================================================================== */

int	main(void)
{
	printf(CYAN BOLD "\n===== PART 1 : LIBC =====\n" RESET);
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
	printf(CYAN BOLD "\n===== PART 2 : ADDITIONAL FUNCTIONS =====\n" RESET);
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
	printf(CYAN BOLD "\n===== PART 3 : LINKED LISTS =====\n" RESET);
	lstnew_test();
	lstadd_front_test();
	lstsize_test();
	lstlast_test();
	lstadd_back_test();
	lstdelone_test();
	lstclear_test();
	lstiter_test();
	lstmap_test();
	printf(CYAN BOLD "\n===== MALLOC FAILURES (NULL return; for leaks: make asan / make vg) =====\n" RESET);
	malloc_fail_test();
	printf(CYAN BOLD "\n===== EDGE CASES : should not crash =====\n" RESET);
	edge_cases();
	printf(CYAN BOLD "\n===== UNDEFINED BEHAVIOUR : indicative, nothing is required =====\n" RESET);
	undefined_cases();
	printf("\n%s " CYAN "(%d tests failure)\n\n" RESET,
		g_fail ? BOLD "RESULT : " RED "KO" RESET : BOLD "RESULT : " GREEN "ALL GOOD" RESET, g_fail);
	return (g_fail != 0);
}