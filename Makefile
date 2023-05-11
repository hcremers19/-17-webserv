############################################
################# MAKEFILE #################
############################################

# -.-.-.-.	Colors 	-.-.-.-.-.

C_DEFAUT		=	\033[0;39m
C_ORANGE		=	\033[0;33m
C_GREEN 		= 	\033[1;32m
C_RED			= 	\033[0;31m

#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-

NAME = webserv

#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-#

FLAGS 	= 	-Wall -Wextra -Werror -std=c++11 -I includes							# flag c++98
CC		= 	c++
RM		= 	rm -f
PRI		= 	printf
VEL		= 	sleep

#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-#

INCLUDES	=	./includes

SRCS		=	./srcs/main.cpp 			\
				./srcs/Client.class.cpp 	\
				./srcs/Requete.class.cpp \
				./srcs/Conf.class.cpp	\
				./srcs/Server.class.cpp	\
				./srcs/Socket.class.cpp 	\
				./srcs/cgi.cpp 			\
				./srcs/Servers.class.cpp \
				./srcs/Location.class.cpp 

#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-#

all : ${NAME}

${NAME} :
#	@${PRI} "\n${C_ORANGE}... compiling ...\n${C_DEFAUT}"
	@${CC} ${FLAGS} -I ${INCLUDES} ${SRCS} -o ${NAME}
	@${PRI} "${C_ORANGE}... compiling ...\n\n${C_DEFAUT}"
#	@${VEL} 0.3
	@${PRI} "${C_GREEN}$@ exec --> Successfully Build\n\n${C_DEFAUT}"
		
clean:

fclean: clean
	@${RM} ${NAME}
	@${PRI} "\n${C_RED}Exec file deleted\n\n${C_DEFAUT}"

re: fclean all

.PHONY: all fclean clean re