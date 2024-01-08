#include <unistd.h>
#include "webserv.hpp"
#include "CgiResponseModule.hpp"

char CgiResponseHandler::_buf[BUF_SIZE];

void CgiResponseHandler::recvCgiResponse(int fd, size_t size)
{
    size_t recvLen;

    while (size > 0) {
        recvLen = (BUF_SIZE < size) ? BUF_SIZE : size;
        size -= recvLen;
        if (read(fd, _buf, recvLen) == FAILURE)
            throw ERROR;
        _rawCgiResponse.append(_buf, recvLen);
    }
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
