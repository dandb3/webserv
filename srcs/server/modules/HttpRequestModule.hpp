#ifndef HTTP_REQUEST_MODULE_HPP
#define HTTP_REQUEST_MODULE_HPP

#include <queue>
#include <string>
#include <unistd.h>
#include <vector>
#include "../../webserv.hpp"
#include "../cycle/ICycle.hpp"
#include "HttpRequest.hpp"

class HttpRequestHandler
{
private:
    enum
    {
        INPUT_READY,
        INPUT_REQUEST_LINE,
        INPUT_HEADER_FIELD,
        INPUT_MESSAGE_BODY,
        INPUT_DEFAULT_BODY,
        INPUT_CHUNKED_BODY,
        PARSE_FINISHED,
        INPUT_NORMAL_CLOSED,
        INPUT_ERROR_CLOSED
    };

    char _status;
    size_t _contentLength;

    std::string _remain;
    std::vector<std::string> _lineV;

    HttpRequest _httpRequest;
    HttpRequest _cycleHttpRequest;

    void _inputEOF();
    void _inputStart();

    void _inputRequestLine();
    void _parseRequestLine();

    void _inputHeaderField();
    void _parseQuery(RequestLine &requestLine, std::string &query);
    void _parseHeaderField();

    void _extractContentLength(int contentLengthCount);
    void _inputMessageBody();
    void _inputDefaultBody();
    void _inputChunkedBody();

    void _pushRequest(std::queue<HttpRequest> &httpRequestQ);
public:
    enum
    {
        METHOD_ERROR = -1,
        GET,
        HEAD,
        POST,
        DELETE
    };
    HttpRequestHandler();

    void recvHttpRequest(int fd, size_t size);
    void parseHttpRequest(bool eof, std::queue<HttpRequest> &httpRequestQ);

    HttpRequest& getHttpRequest();
    void setHttpRequest(const HttpRequest& httpRequest);

    bool isInputReady() const;
    bool closed() const;

    void reset();

};

#endif
