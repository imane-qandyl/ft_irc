NAME = ircserv

CXXFLAGS = -std=c++98 -Wall -Wextra -Werror -Iheaders

SRC = src/main.cpp\
	 src/server.cpp\
	 src/client.cpp\
	 src/signals.cpp


OBJ = $(SRC:.cpp=.o)

CXX = c++

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

all: $(NAME)


clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re