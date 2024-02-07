#include "CgiRequestModule.hpp"

CgiRequest::CgiRequest()
: _metaVariables(), _messageBody()
{}

CgiRequest& CgiRequest::operator=(const CgiRequest& cgiRequest)
{
    if (this == &cgiRequest)
        return *this;

    _metaVariables = cgiRequest._metaVariables;
    _messageBody = cgiRequest._messageBody;
    return *this;
}

void CgiRequest::addMetaVariable(const std::string& key, const std::string& value)
{
    _metaVariables.push_back(key + "=" + value);
}

void CgiRequest::setMessageBody(const std::string& messageBody)
{
    _messageBody = messageBody;
}

std::vector<std::string>& CgiRequest::getMetaVariables()
{
    return _metaVariables;
}

std::string& CgiRequest::getMessageBody()
{
    return _messageBody;
}
