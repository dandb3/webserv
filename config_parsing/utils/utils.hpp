
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include "../ServerConfig.hpp"

#define WHITESPACE " \t\n\r\f\v"
#define DELIMITER " \t\n\r\f\v;"
#define SEPARATOR " \t\n\r\f\v{};"

std::string getWord(std::string const &file_content, size_t &i);
std::string getKey(std::string const &file_content, size_t &i);
std::vector<std::string> getValues(std::string const &file_content, size_t &i, char delimiter);
std::string getValue(std::string const &file_content, size_t &i, char delimiter);

int ft_inet_aton(const char *str, struct in_addr *addr);

void printParsedServer(ServerConfig &server);

// config 파싱 부분에서 getword-> getKey/getValue로 나누기
