#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <vector>
#include <map>
#include "../parse/parse.hpp"

class RequestLine
{
private:
    short _method;
    std::string _uri;
    std::vector<std::pair<std::string, std::string> > _query;
    std::string _fragment;
    std::pair<short, short> _version;

public:
    void setMethod(short method);
    void setUri(std::string uri);
    void setQuery(std::vector<std::pair<std::string, std::string> > &query);
    void setFragment(std::string fragment);
    void setVersion(std::pair<short, short> version);

    short getMethod() const;
    const std::string &getUri() const;
    const std::vector<std::pair<std::string, std::string> > &getQuery() const;
    const std::string &getFragment() const;
    const std::pair<short, short> &getVersion() const;
};

class HttpRequest
{
private:
    unsigned short _code;
    RequestLine _requestLine;
    std::multimap<std::string, std::string> _headerFields;
    std::string _messageBody;

public:
    HttpRequest();
    HttpRequest(unsigned short code);
    HttpRequest(RequestLine &requestLine,
                std::multimap<std::string, std::string> &headerFields, std::string &messageBody);

    void setCode(unsigned short code);
    void setRequestLine(RequestLine &requestLine);
    void setHeaderFields(std::multimap<std::string, std::string> &headerFields);
    void setMessageBody(std::string &messageBody);

    unsigned short getCode() const;
    RequestLine &getRequestLine();
    std::multimap<std::string, std::string> &getHeaderFields();
    std::string &getMessageBody();
};

#endif
