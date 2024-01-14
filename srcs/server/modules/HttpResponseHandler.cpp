#include "HttpResponseModule.hpp"

void HttpResponseHandler::_makeStatus()
{
    StatusLine statusLine;
    short code;

    statusLine.setVersion(make_pair(1, 1));
    statusLine.setCode(code);

    switch (code) {
        case 100:
            statusLine.setText("Continue");
            break;
        case 101:
            statusLine.setText("Switching Protocol");
            break;
        case 102:
            statusLine.setText("Processing");
            break;
        case 200:
            statusLine.setText("OK");
            break;
        case 201:
            statusLine.setText("Created");
            break;
        case 202:
            statusLine.setText("Accepted");
            break;
        case 203:
            statusLine.setText("Non-Authoritative Information");
            break;
        case 204:
            statusLine.setText("No Content");
            break;
        case 404:
            statusLine.setText("Not Found");
            break;
        case 503:
            statusLine.setText("Service Unavailable");
            break;
        default:
            statusLine.setText("Not Yet Setted\n");
    }
}

void HttpResponseHandler::makeHttpResponse(HttpRequest &httpRequest, CgiResponse &cgiResponse, NetConfig &netConfig)
{
    short method;

    // if else -> switch?
    if (method == GET || method == HEAD) {
        
    }
    else if (method == PUT) {

    }
    else if (method == DELETE) {

    }
    else {

    }
}

void HttpResponse::sendHttpResponse(int fd, size_t size)
{
    size_t writeLen;

    writeLen = std::min(_response.size() - _pos, size);
    if (write(fd, _response.c_str() + _pos, writeLen) == FAILURE)
        throw err_syscall();
    _pos += writeLen;
    if (_pos == size) {
        _status = RES_IDLE;
        _pos = 0;
    }
}