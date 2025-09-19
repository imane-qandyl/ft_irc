NAME = ircserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
SRCDIR = src
HEADERDIR = headers
OBJDIR = obj

# Find all .cpp files recursively in src directory
SRCS = $(shell find $(SRCDIR) -name "*.cpp")
OBJS = $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Find all header directories recursively
HEADER_DIRS = $(shell find $(HEADERDIR) -type d)
INCLUDE_FLAGS = $(addprefix -I, $(HEADER_DIRS))

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re