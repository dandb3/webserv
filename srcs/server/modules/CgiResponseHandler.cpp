#include <algorithm>
#include <unistd.h>
#include "webserv.hpp"
#include "CgiResponseModule.hpp"

char CgiResponseHandler::_buf[BUF_SIZE];

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
    size_t recvLen;

    if ((recvLen = read(kev.ident, _buf, std::min<size_t>(BUF_SIZE, kev.data))) == FAILURE)
        throw 500;
    _rawCgiResponse.append(_buf, recvLen);
    if ((kev.flags & EV_EOF) && kev.data == 0)
        _eof = true;
}

void CgiResponseHandler::makeCgiResponse()
{
    CgiResponseParser::parseCgiResponse(_cgiResponse, _rawCgiResponse);
}

const CgiResponse& CgiResponseHandler::getCgiResponse() const
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
