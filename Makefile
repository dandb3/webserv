# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sunwsong <sunwsong@student.42seoul.kr>     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/11/19 17:38:04 by sunwsong          #+#    #+#              #
#    Updated: 2024/01/18 19:24:48 by sunwsong         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS		=	test_srv.cpp ./srcs/server/modules/Http*.cpp \
				./srcs/server/modules/*Line.cpp ./srcs/server/cycle/ConfigInfo.cpp \
				./srcs/exception/Exception.cpp

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
