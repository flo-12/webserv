

# Executable name
NAME		= webserv

# Compiler
CC			= c++
FLAGS		= -Wextra -Wall -Werror -std=c++98

# includes
INC			=	-I ./includes/

# Sources & Objects
SRC_DIR		= ./srcs/
SRC_FILES	= main Webserv Socket ClientSocket ServerSocket RequestParser
#SRC_FILES	+=  CGIHandler ConfigParser ServerConfig ServerLocation utils
SRCS		= $(addprefix $(SRC_DIR),$(addsuffix .cpp,$(SRC_FILES)))

OBJ_DIR		= ./.objFiles/
OBJ_FILES	= $(addsuffix .o,$(SRC_FILES))
OBJS		= $(addprefix $(OBJ_DIR), $(OBJ_FILES))

HEADER_DIR	= ./includes/
HEADER_FILES= Webserv Socket ClientSocket ServerSocket RequestParser ConfigParser ServerConfig ServerConfig common
HEADER		= $(addprefix $(HEADER_DIR),$(addsuffix .hpp,$(HEADER_FILES)))

# RULES
all: $(OBJ_DIR) $(NAME)

$(NAME): $(OBJS)
	@echo "Compiling $(NAME)..."
	@$(CC) $(OBJS) -o $(NAME)
	@echo "\n\t\033[33;1;3m$(NAME) ready\033[0m\n"

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(FLAGS) -c $< -o $@ $(INC)

$(OBJ_DIR):
	@echo "Generating obj directory..."
	@mkdir $(OBJ_DIR)

clean:
	@echo "Removing object files..."
	@rm -rf $(OBJ_DIR) $(OBJS)

fclean: clean
	@echo "Removing executable and object files..."
	@rm -f $(NAME)

re: fclean all

.PHONY: all re clean fclean
