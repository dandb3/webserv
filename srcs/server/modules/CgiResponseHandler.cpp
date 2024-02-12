#include <algorithm>
#include <unistd.h>
#include "CgiResponseModule.hpp"

CgiResponseHandler::CgiResponseHandler()
: _cgiResponse(), _rawCgiResponse(), _eof(false)
{}

CgiResponseHandler& CgiResponseHandler::operator=(const CgiResponseHandler& cgiResponseHandler)
{
    if (this == &cgiResponseHandler)
        return *this;

    _cgiResponse = cgiResponseHandler._cgiResponse;
    _rawCgiResponse = cgiResponseHandler._rawCgiResponse;
    _eof = cgiResponseHandler._eof;
    return *this;
}

void CgiResponseHandler::recvCgiResponse(const struct kevent& kev)
{
    ssize_t recvLen;

    if ((recvLen = read(kev.ident, ICycle::getBuf(), std::min<size_t>(BUF_SIZE, kev.data))) == FAILURE)
        throw 500;
    _rawCgiResponse.append(ICycle::getBuf(), static_cast<size_t>(recvLen));
    if ((kev.flags & EV_EOF) && kev.data == 0)
        _eof = true;
}

void CgiResponseHandler::makeCgiResponse()
{
    CgiResponseParser::parseCgiResponse(_cgiResponse, _rawCgiResponse);
}

CgiResponse& CgiResponseHandler::getCgiResponse()
{
    return _cgiResponse;
}

char CgiResponseHandler::getResponseType() const
{
    return _cgiResponse.getType();
}

bool CgiResponseHandler::eof() const
{
    return _eof;
}

void CgiResponseHandler::reset()
{
    _cgiResponse.setStatusCode(0);
    _cgiResponse.getHeaderFields().clear();
    _cgiResponse.getMessageBody().clear();
    _rawCgiResponse.clear();
    _eof = false;
}
