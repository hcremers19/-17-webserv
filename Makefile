############################################
################# MAKEFILE #################
############################################

# -.-.-.-.	Colors 	-.-.-.-.-.

C_DEFAULT		=	\033[0;39m
C_YELLOW		=	\033[0;33m
C_GREEN 		= 	\033[1;32m
C_RED			= 	\033[0;31m

#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-

NAME = webserv

#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-#

FLAGS 	= 	-Wall -Wextra -Werror -std=c++98 -I includes
CC		= 	c++
RM		= 	rm -f
PRI		= 	printf
VEL		= 	sleep

#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-#

INCLUDES	=	./includes

SRCS		=	Sources/CGI/CGI.cpp	\
				Sources/Config/Config.cpp	\
				Sources/Host/Host.cpp	\
				Sources/HTML/Client.cpp	\
				Sources/HTML/Request.cpp	\
				Sources/Server/Location.cpp	\
				Sources/Server/Server.cpp	\
				Sources/Socket/Socket.cpp	\
				Sources/main.cpp

#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-#

all: ${NAME}

${NAME}:
#	@${PRI} "\n${C_YELLOW}... compiling ...\n${C_DEFAULT}"
	@${PRI} "${C_YELLOW}... compiling ...\n\n${C_DEFAULT}"
	@${CC} ${FLAGS} -I ${INCLUDES} ${SRCS} -o ${NAME}
#	@${VEL} 0.3
	@${PRI} "${C_GREEN}$@ exec --> Successfully Built\n${C_DEFAULT}"
		
clean:
	@${RM} ${NAME}
	@${PRI} "${C_RED}Exec file deleted\n\n${C_DEFAULT}"

fclean: clean

re: fclean all

.PHONY: all fclean clean re