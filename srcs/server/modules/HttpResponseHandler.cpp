#include "HttpResponseModule.hpp"

void HttpResponseHandler::_makeStatusLine(StatusLine &statusLine, short code)
{
    std::string text;

    statusLine.setVersion(make_pair(1, 1));
    statusLine.setCode(code);

    switch (code) {
        case 100:
            text = "Continue";
            break;
        case 101:
            text = "Switching Protocol";
            break;
        case 102:
            text = "Processing";
            break;
        case 200:
            text = "OK";
            break;
        case 201:
            text = "Created";
            break;
        case 202:
            text = "Accepted";
            break;
        case 203:
            text = "Non-Authoritative Information";
            break;
        case 204:
            text = "No Content";
            break;
        case 404:
            text = "Not Found";
            break;
        case 503:
            text = "Service Unavailable";
            break;
        default:
            text = "Not Yet Setted\n";
    }

    statusLine.setText(text);
}

void HttpResponseHandler::_makeGETResponse(HttpRequest &httpRequest, NetConfig &netConfig, bool isGET)
{
    const int fileFd = open(netConfig.getPath());
    StatusLine statusLine;
    std::multimap<std::string, std::string> headerFields;
    std::string messageBody;

    if (fileFd == -1) {
        _makeStatusLine(statusLine, 404);
        _httpResponse.setStatusLine(statusLine);
        return;
    }

    // 여러가지 상황에 따라 code를 세팅한다.
    // 아직 미구현 사항이며, 우선적으로 200 OK 혹은 404 Not Found를 보내는 것을 목표로 구현.
    _makeStatusLine(statusLine, 200);

    if (isGET) {
        // fildFd로부터 해당 파일을 읽어온다.
        // std::string에 append 혹은 push_back을 통해서 body를 만든다.(C++ version 확인)
        // HTTPresponse의 messagebody에 할당해준다.
        // method가 HEAD인 경우에 body를 세팅하지 않는다.
    }

    // Header Field들을 세팅해준다.
    // _makeHeaderFields();
    headerFields["Content-Length"] = strtol(messageBody);

    _httpResponse.setStatusLine(statusLine);
    _httpResponse.setHeaderFields(headerFields);
    _httpResponse.setMessageBody(messageBody);
}

void HttpResponseHandler::makeHttpResponse(HttpRequest &httpRequest, NetConfig &netConfig)
{
    const short method = httpRequest.getRequestLine().getMethod();

    // if else -> switch?
    if (method == GET || method == HEAD) {
        _makeGETResponse(httpRequest, netConfig, (method == GET));
    }
    else if (method == PUT) {
        _makePUTResponse();
    }
    else if (method == DELETE) {
        _makeDELETEResponse();
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