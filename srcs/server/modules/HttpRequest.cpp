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

const RequestLine& HttpRequest::getRequestLine() const
{
    return _requestLine;
}

std::multimap<std::string, std::string>& HttpRequest::getHeaderFields() const
{
    return _headerFields;
}

const std::string HttpRequest::getMessageBody() const
{
    return _messageBody;
}
