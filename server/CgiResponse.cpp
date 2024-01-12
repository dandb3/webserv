#include "CgiResponseModule.hpp"

CgiResponse::CgiResponse()
{}

CgiResponse::CgiResponse(const std::string& rawCgiResponse)
{}

CgiResponse& CgiResponse::operator=(const CgiResponse& cgiResponse)
{
    if (this == &cgiResponse)
        return *this;

    // copy header-fields
    // copy message-body
    return *this;
}

void CgiResponse::addHeaderField(const std::string& key, const std::string& value)
{
    _headerFields.insert(std::make_pair(key, value));
}

void CgiResponse::setMessageBody(const std::string& messageBody)
{
    _messageBody = messageBody;
}

const std::map<std::string, std::string>& CgiResponse::getHeaderFields() const
{
    return _headerFields;
}

const std::string& CgiResponse::getMessageBody() const
{
    return _messageBody;
}
