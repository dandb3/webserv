#include "CgiResponseModule.hpp"

CgiResponse::CgiResponse()
{}

CgiResponse::CgiResponse(unsigned short code)
: _statusCode(code)
{}

CgiResponse::CgiResponse(const std::string& rawCgiResponse)
{
    _messageBody = rawCgiResponse;
}

CgiResponse& CgiResponse::operator=(const CgiResponse& cgiResponse)
{
    if (this == &cgiResponse)
        return *this;

    _headerFields = cgiResponse._headerFields;
    _messageBody = cgiResponse._messageBody;
    _type = cgiResponse._type;
    return *this;
}

void CgiResponse::setStatusCode(unsigned short statusCode)
{
    _statusCode = statusCode;
}

void CgiResponse::addHeaderField(const pair_t& p)
{
    _headerFields.push_back(p);
}

void CgiResponse::setMessageBody(const std::string& messageBody)
{
    _messageBody = messageBody;
}

void CgiResponse::setType(char type)
{
    _type = type;
}

unsigned short CgiResponse::getStatusCode() const
{
    return _statusCode;
}

std::vector<pair_t>& CgiResponse::getHeaderFields()
{
    return _headerFields;
}

std::string& CgiResponse::getMessageBody()
{
    return _messageBody;
}

char CgiResponse::getType() const
{
    return _type;
}
