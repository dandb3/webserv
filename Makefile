# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sunwsong <sunwsong@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/11/19 17:38:04 by sunwsong          #+#    #+#              #
#    Updated: 2024/02/12 19:10:12 by sunwsong         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS		=	./srcs/config/ServerConfig.cpp \
				./srcs/config/LocationConfig.cpp \
				./srcs/config/Config.cpp \
				./srcs/config/ConfigParser.cpp \
				./srcs/server/KqueueHandler.cpp \
				./srcs/server/ServerManager.cpp \
				./srcs/server/EventHandler.cpp \
				./srcs/server/cycle/ICycle.cpp \
				./srcs/server/cycle/Cycle.cpp \
				./srcs/server/cycle/WriteFile.cpp \
				./srcs/server/cycle/ConfigInfo.cpp \
				./srcs/server/parse/parse.cpp \
				./srcs/server/parse/CgiResponseParser.cpp \
				./srcs/server/modules/CgiRequest.cpp \
				./srcs/server/modules/HttpResponseHandler.cpp \
				./srcs/server/modules/CgiResponse.cpp \
				./srcs/server/modules/CgiResponseHandler.cpp \
				./srcs/server/modules/post.cpp \
				./srcs/server/modules/CgiRequestHandler.cpp \
				./srcs/server/modules/HttpRequest.cpp \
				./srcs/server/modules/RequestLine.cpp \
				./srcs/server/modules/HttpRequestHandler.cpp \
				./srcs/utils/utils.cpp \
				./srcs/utils/FileReader.cpp \
				./srcs/main.cpp

OBJS		= ${SRCS:.cpp=.o}

CXXFLAGS	= -std=c++98 -Wall -Wextra -Werror

CGI			= var/script/cat
CGI_OBJ		= srcs/script/basicCgi.o

NAME		= srv

DEBUG_MODE	= $(findstring debug, $(MAKECMDGOALS))
DEFINE		= $(if $(DEBUG_MODE), -D DEBUG)

all:	${NAME}

debug:	${NAME}

clean:
		${RM} ${OBJS} ${CGI_OBJ}

fclean:	clean
		${RM} ${NAME} ${CGI}

re:		fclean all

${NAME}:	${OBJS} ${CGI}
		${CXX} ${CXXFLAGS} ${DEFINE} ${OBJS} -o $@

${CGI}: ${CGI_OBJ}
		${CXX} ${CXXFLAGS} ${DEFINE} ${CGI_OBJ} -o $@

%.o : %.cpp
			${CXX} ${CXXFLAGS} ${DEFINE} -o $@ -c $<

.PHONY: all clean fclean re debug
