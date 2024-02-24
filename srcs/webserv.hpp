#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <netinet/in.h>

#define FAILURE -1
#define SUCCESS 0

#define CRLF "\r\n"
#define WHITESPACE " \t\n\r\f\v"

#define ERROR 1

#ifndef DEBUG
#define log(...)
#else
#include "debug.hpp"
#define log(...) LOG(__VA_ARGS__)
#endif

#endif
