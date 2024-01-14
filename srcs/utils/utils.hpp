#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../config/ServerConfig.hpp"

#define WHITESPACE " \t\n\r\f\v"
#define DELIMITER " \t\n\r\f\v;"
#define SEPARATOR " \t\n\r\f\v{};"

std::string sizeToStr(size_t size);
std::string getWord(std::string const &fileContent, size_t &i);
std::string getKey(std::string const &fileContent, size_t &i);
std::vector<std::string> getValues(std::string const &fileContent, size_t &i, char delimiter);
std::string getValue(std::string const &fileContent, size_t &i, char delimiter);

std::string ft_inet_ntoa(in_addr_t addr);
int ft_inet_aton(const char *str, struct in_addr *addr);

void printParsedServer(ServerConfig &server);
