NAME = myftp

SRCS = main.c cwd.c client.c listen.c local.c socket.c

OBJS = $(SRCS:.c=.o)

CFLAGS = -Werror -Wall

all: $(NAME)

$(NAME): $(OBJS)
	 gcc -o $(NAME) $(OBJS)

%.o: %.c
	gcc -c $< $(CFLAGS)

clean:
	rm -rf $(OBJS)
	rm -rf temp/*

fclean: clean
	rm -rf $(NAME)

re:	fclean all
