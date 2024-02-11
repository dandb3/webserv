# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sunwsong <sunwsong@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/11/19 17:38:04 by sunwsong          #+#    #+#              #
#    Updated: 2024/01/30 17:40:20 by sunwsong         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS		=	test_srv.cpp \
				./srcs/server/modules/RequestLine.cpp \
				./srcs/server/modules/HttpRequest.cpp \
				./srcs/server/modules/HttpRequestHandler.cpp \
				./srcs/server/parse/parse.cpp \

OBJS		= ${SRCS:.cpp=.o}

CXXFLAGS	= -Wall -Wextra -Werror -std=c++98

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
