#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <sstream>
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

std::string getWord(std::string const &fileContent, size_t &i);
std::string getKey(std::string const &fileContent, size_t &i);
std::vector<std::string> getValues(std::string const &fileContent, size_t &i, char delimiter);
std::string getValue(std::string const &fileContent, size_t &i, char delimiter);

std::string sizeToStr(size_t size);
std::string ft_itoa(int num);
std::string ft_inet_ntoa(in_addr_t addr);
int ft_inet_aton(const char *str, struct in_addr *addr);

template <typename T>
T stringToType(const std::string& str)
{
    std::stringstream ss(str);
    T result;

    ss >> result;
    return result;
}

template <typename T>
std::string toString(const T& val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

void printParsedServer(ServerConfig &server);

std::vector<std::pair<std::string, std::string> > &parseQuery(const std::string &query);
std::vector<std::string> &splitByComma(const std::string &str);
std::string &decodeUrl(const std::string &str);

#endif