CC = gcc
CFLAGS = -Wall -Wextra -Werror
INCDIR = ./
NAME = pipex
OBJS = pipex.o

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

.c.o:
	$(CC) $(CFLAGS) -I $(INCDIR) -c $<

all: $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
