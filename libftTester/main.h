#ifndef MAIN_H
# define MAIN_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <limits.h>
# include <stdint.h>
# include <bsd/string.h>
# include <ctype.h>
# include "../libft.h"

int	g_fail = 0;

# define T(e) do { if (e) printf("OK "); \
	else { printf("KO[l.%d] ", __LINE__); g_fail++; } } while (0)
# define START(n) printf("%-14s: ", n)
# define END() printf("\n")

#endif