NAME		=	libft-tester
CC			=	cc
FLAGS		=	-Wall -Wextra -Werror -g
LDFLAGS		=	-Wl,--wrap=malloc
VG			=	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind_%p.log
LIBFT_DIR	=	../
HEADERS		=	main.h $(LIBFT_DIR)/libft.h
INCDIR		=	-I $(LIBFT_DIR) -I .
RM			=	rm -rf

# make SAN=1 : compile libft ET le testeur avec AddressSanitizer + UBSan
# (a utiliser avec "make re SAN=1", puis refaire un "make fclean" ensuite)
ifdef SAN
FLAGS		+=	-fsanitize=address,undefined
endif

SRCS		=	main.c
OBJS		=	$(SRCS:.c=.o)

%.o:			%.c $(HEADERS)
				@$(CC) $(FLAGS) $(INCDIR) -c $< -o $@

all:			$(NAME)

# libft est recompilee avec les MEMES flags que le testeur, sinon ASan ne
# verrait pas les erreurs commises a l interieur de libft.a
$(NAME): 		libft $(OBJS)
				@$(RM) valgrind_*.log
				@$(CC) $(FLAGS) $(OBJS) -L $(LIBFT_DIR) -lft $(LDFLAGS) -o $(NAME)

libft:
				@$(MAKE) -C $(LIBFT_DIR) re FLAGS="$(FLAGS)" -s

run:			all
				@./$(NAME)
				@$(MAKE) -s fclean

asan:
				@$(RM) valgrind_*.log
				@$(MAKE) re SAN=1 -s
				@./$(NAME)
				@$(MAKE) -s fclean

vg:				all
				@$(VG) ./$(NAME)
				@for f in valgrind_*.log; do \
					if [ -f "$$f" ]; then \
						if ! grep -q "definitely lost: [1-9]" "$$f" && \
						   ! grep -q "indirectly lost: [1-9]" "$$f" && \
						   ! grep -q "depends on uninitialised value" "$$f" && \
						   ! grep -q "Invalid free" "$$f"; then \
							rm -f "$$f"; \
						fi; \
					fi; \
				done
				@$(MAKE) -s fclean

clean:
				@$(MAKE) clean -C $(LIBFT_DIR) -s
				@$(RM) $(OBJS)

fclean:			clean
				@$(MAKE) fclean -C $(LIBFT_DIR) -s
				@$(RM) $(NAME)

vclean:			fclean
				@$(RM) valgrind_*.log

re:				vclean all

.PHONY:			all libft run asan vg clean fclean vclean re