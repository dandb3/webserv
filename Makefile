# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sunwsong <sunwsong@student.42seoul.kr>     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/11/19 17:38:04 by sunwsong          #+#    #+#              #
#    Updated: 2024/01/18 19:30:06 by sunwsong         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS		=	test_srv.cpp\
				./srcs/server/modules/RequestLine.cpp \
				./srcs/server/modules/StatusLine.cpp \
				./srcs/server/modules/HttpRequest.cpp \
				./srcs/server/modules/HttpResponse.cpp \
				./srcs/server/modules/HttpRequestHandler.cpp \
				./srcs/server/modules/HttpResponseHandler.cpp \
				./srcs/server/modules/HttpRequestModule.hpp	\
				./srcs/server/modules/HttpResponseModule.hpp \
				./srcs/server/cycle/ConfigInfo.cpp \
				./srcs/exception/Exception.cpp

OBJS		= ${SRCS:.cpp=.o}

CXXFLAGS	= -std=c++98

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
