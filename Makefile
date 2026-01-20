NAME = ircserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp SRC/Authenti.cpp SRC/Channel.cpp SRC/Client.cpp SRC/Server.cpp \
	CMD/INVITE.cpp CMD/JOIN.cpp CMD/KICK.cpp CMD/MODE.cpp CMD/PART.cpp CMD/PRIVMSG.cpp CMD/QUIT.cpp CMD/TOPIC.cpp 

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp INC/Server.hpp INC/Client.hpp INC/Channel.hpp INC/replies.hpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re