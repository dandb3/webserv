#include "HttpRequestModule.hpp"

HttpRequest::HttpRequest()
: _code(0)
{}

HttpRequest::HttpRequest(RequestLine &requestLine, \
        std::multimap<std::string, std::string> &headerFields, std::string &messageBody) 
        : _requestLine(requestLine), _headerFields(headerFields), _messageBody(messageBody) {}

void HttpRequest::setCode(unsigned short code)
{
    _code = code;
}

void HttpRequest::setRequestLine(RequestLine &requestLine)
{
    _requestLine = requestLine;
}

void HttpRequest::setHeaderFields(std::multimap<std::string, std::string> &headerFields)
{
    std::multimap<std::string, std::string>::iterator iter = headerFields.begin();

    for (; iter != headerFields.end(); iter++) {
        _headerFields.insert(*iter);
    }
}

void HttpRequest::setMessageBody(std::string &messageBody)
{
    _messageBody = messageBody;
}

unsigned short HttpRequest::getCode() const
{
    return _code;
}

RequestLine& HttpRequest::getRequestLine()
{
    return _requestLine;
}

std::multimap<std::string, std::string>& HttpRequest::getHeaderFields()
{
    return _headerFields;
}

std::string &HttpRequest::getMessageBody()
{
    return _messageBody;
}
