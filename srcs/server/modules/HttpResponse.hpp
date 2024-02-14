#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <map>

class StatusLine
{
public:
    std::pair<short, short> version;
    unsigned short code;
    std::string text;

};

class HttpResponse
{
public:
    StatusLine statusLine;
    std::multimap<std::string, std::string> headerFields;
    std::string messageBody;
};

#endif
