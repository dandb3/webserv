#ifndef HTTP_REQUEST_MODULE_HPP
#define HTTP_REQUEST_MODULE_HPP

#include <queue>
#include <string>
#include <map>
#include <unistd.h>
#include <vector>
#include "../cycle/Cycle.hpp"
#include "../../webserv.hpp"

class Cycle;

class RequestLine
{
private:
    short _method;
    std::string _uri;
    std::vector<std::pair<std::string, std::string> > _query;
    std::string _fragment;
    std::pair<short, short> _version;

public:
    void setMethod(short method);
    void setUri(std::string uri);
    void setQuery(std::vector<std::pair<std::string, std::string> > &query);
    void setFragment(std::string fragment);
    void setVersion(std::pair<short, short> version);

    short getMethod() const;
    const std::string &getUri() const;
    const std::vector<std::pair<std::string, std::string> > &getQuery() const;
    const std::string &getFragment() const;
    const std::pair<short, short> &getVersion() const;
};

class HttpRequest
{
private:
    unsigned short _code;
    RequestLine _requestLine;
    std::multimap<std::string, std::string> _headerFields;
    std::string _messageBody;

public:
    HttpRequest();
    HttpRequest(unsigned short code);
    HttpRequest(RequestLine &requestLine,
                std::multimap<std::string, std::string> &headerFields, std::string &messageBody);

    void setCode(unsigned short code);
    void setRequestLine(RequestLine &requestLine);
    void setHeaderFields(std::multimap<std::string, std::string> &headerFields);
    void setMessageBody(std::string &messageBody);

    unsigned short getCode() const;
    RequestLine &getRequestLine();
    std::multimap<std::string, std::string> &getHeaderFields();
    std::string &getMessageBody();
};

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
    size_t _clientMaxBodySize;

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

};

#endif
