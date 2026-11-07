#ifndef MAIN_H
# define MAIN_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <limits.h>
# include <stdint.h>
# include <ctype.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/wait.h>
# include "libft.h"

extern int	g_fail;

/* Codes couleurs ANSI */
# define RESET   "\033[0m"
# define RED     "\033[31m"
# define GREEN   "\033[32m"
# define YELLOW  "\033[33m"
# define ORANGE  "\033[38;5;208m"
# define CYAN    "\033[36m"
# define BOLD    "\033[1m"
# define MAGENTA "\033[35m"

# define T(e) do { if (e) printf(GREEN "OK " RESET); \
	else { printf(RED "KO[l.%d] " RESET, __LINE__); g_fail++; } } while (0)
# define START(n) printf("%-14s: ", n)
# define END() printf("\n")

#endif