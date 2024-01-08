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

const std::vector<std::string>& CgiRequest::getMetaVariables() const
{
    return _metaVariables;
}

const std::string& CgiRequest::getMessageBody() const
{
    return _messageBody;
}
