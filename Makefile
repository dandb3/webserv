# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sunwsong <sunwsong@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/11/19 17:38:04 by sunwsong          #+#    #+#              #
#    Updated: 2024/02/11 14:43:13 by sunwsong         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS		=	./srcs/config/ServerConfig.cpp \
				./srcs/config/LocationConfig.cpp \
				./srcs/config/Config.cpp \
				./srcs/config/ConfigParser.cpp \
				./srcs/server/KqueueHandler.cpp \
				./srcs/server/ServerManager.cpp \
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
				./srcs/server/modules/StatusLine.cpp \
				./srcs/server/modules/RequestLine.cpp \
				./srcs/server/modules/HttpRequestHandler.cpp \
				./srcs/server/EventHandler.cpp \
				./srcs/utils/utils.cpp \
				./srcs/utils/FileReader.cpp \
				./srcs/exception/Exception.cpp \
				./srcs/main.cpp

OBJS		= ${SRCS:.cpp=.o}

CXXFLAGS	= -std=c++98 #-Wall -Wextra -Werror 

NAME		= srv

all:	${NAME}

clean:
		${RM} ${OBJS}

fclean:	clean
		${RM} ${NAME}

re:		fclean all

${NAME}:	${OBJS}
		${CXX} ${CXXFLAGS} ${OBJS} -o $@

%.o : %.cpp
			${CXX} ${CXXFLAGS} -o $@ -c $<

.PHONY: all clean fclean re
