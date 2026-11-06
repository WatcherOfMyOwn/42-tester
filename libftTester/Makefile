NAME		=	libft-tester
CC			=	gcc
VG			=	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes
FLAGS		=	
LIBFT_DIR	=	../
HEADERS		=	main.h $(LIBFT_DIR)/libf.h
INCDIR		=	-I $(LIBFT_DIR) -I .
RM			=	rm -rf

SRCS		=	main.c
OBJS 		=	$(SRCS:.c=.o)

%.o:			%.c $(HEADERS)
				$(CC) $(FLAGS) $(INCDIR) -c $< -o $@

all:			$(NAME)

$(NAME): 		libft $(OBJS)
				$(CC) $(FLAGS) $(OBJS) -L $(LIBFT_DIR) -lbsd -lft -o $(NAME)

vg:				all
				$(VG) ./$(NAME)

libft:
				$(MAKE) -C $(LIBFT_DIR)

clean:
				$(MAKE) clean -C $(LIBFT_DIR)
				$(RM) $(OBJS)

fclean:			clean
				$(MAKE) fclean -C $(LIBFT_DIR)
				$(RM) $(NAME)

re:				fclean all

.PHONY:			all libft clean fclean re