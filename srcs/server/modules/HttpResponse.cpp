#include <algorithm>
#include <unistd.h>
#include "HttpResponseModule.hpp"

void HttpResponse::setStatusLine(StatusLine &statusLine)
{
    _statusLine = statusLine;

}

void HttpResponse::setHeaderFields(std::multimap<std::string, std::string> &headerFields)
{
    std::multimap<std::string, std::string>::iterator iter = headerFields.begin();

    for (; iter != headerFields.end(); iter++) {
        _headerFields.insert(*iter);
    }
}

void HttpResponse::setMessageBody(std::string &messageBody)
{
    _messageBody = messageBody;
}

const StatusLine& HttpResponse::getStatusLine() const
{
    return _statusLine;
}

std::multimap<std::string, std::string>& HttpResponse::getHeaderFields()
{
    return _headerFields;
}

const std::string& HttpResponse::getMessageBody() const
{
    return _messageBody;
}
