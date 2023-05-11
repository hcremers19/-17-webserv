NAME = webserv

FLAGS = -Wall -Wextra -Werror -std=c++11 -I includes							# flag c++98

INCLUDES	=	./includes

SRCS_DIR = src
OBJS_DIR = obj

SRCS =	main.cpp \
		Client.class.cpp \
		Requete.class.cpp \
		Conf.class.cpp\
		Server.class.cpp \
		Socket.class.cpp \
		cgi.cpp \
		Servers.class.cpp \
		Location.class.cpp \

OBJS = $(addprefix ${OBJS_DIR}/,${SRCS:.cpp=.o})

all: create_dir $(OBJS)
		@c++ $(FLAGS) $(OBJS) -o $(NAME)
		@echo "\033[32mCOMPILATION OK\033[0m"

${OBJS_DIR}/%.o:${SRCS_DIR}/%.cpp
		@c++ ${FLAGS} -I ${INCLUDES} -c $< -o $@
		
clean:
		@rm -rf $(OBJS)
		@echo "\033[36mCLEAN OK\033[0m"

fclean: clean
		@rm -rf $(NAME)
		@rm -rf $(OBJS_DIR)
		@echo "\033[36mFCLEAN OK\033[0m"

re: fclean all

create_dir:
		@mkdir -p $(OBJS_DIR)

.PHONY: all fclean clean re