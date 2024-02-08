#ifndef POST_HPP
#define POST_HPP

#include <cstring>
#include <sstream>
#include "HttpResponseModule.hpp"
#include "../../utils/utils.hpp"

std::string parseUrlencode(const std::string &encodedUrl);
void parseTextPlain(std::string &body);
void parseMultiForm(const std::string &contentType, const std::string &body, std::map<std::string, std::string> &files);

#endif