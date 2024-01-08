#include <unistd.h>
#include "webserv.hpp"
#include "CgiResponseModule.hpp"

char CgiResponseHandler::_buf[BUF_SIZE];

CgiResponseHandler::CgiResponseHandler()
: _cgiResponse(), _rawCgiResponse(), _eof(false)
{}

void CgiResponseHandler::recvCgiResponse(struct kevent& kev)
{
    size_t recvLen, totalSize = static_cast<size_t>(kev.data);

    while (totalSize > 0) {
        recvLen = (BUF_SIZE < totalSize) ? BUF_SIZE : totalSize;
        totalSize -= recvLen;
        if (read(kev.ident, _buf, recvLen) == FAILURE)
            throw ERROR;
        _rawCgiResponse.append(_buf, recvLen);
    }
    if (kev.flags & EV_EOF)
        _eof = true;
}

void CgiResponseHandler::makeCgiResponse()
{
    _cgiResponse = CgiResponse(_rawCgiResponse);
    _rawCgiResponse.clear();
}

const CgiResponse& CgiResponseHandler::getCgiResponse() const
{
    return _cgiResponse;
}

bool CgiResponseHandler::eof() const
{
    return _eof;
}
