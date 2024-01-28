#ifndef HTTP_REQUEST_MODULE_HPP
#define HTTP_REQUEST_MODULE_HPP

#include <queue>
#include <string>
#include <map>
#include <unistd.h>
#include <vector>
#include "../../webserv.hpp"

#define BUF_SIZE 1024

class RequestLine
{
private:
    short _method;
    std::string _uri;
    std::vector<std::pair<std::string, std::string> > _query;
    std::pair<short, short> _version;

public:
    RequestLine &operator=(const RequestLine &ref);

    void setMethod(short method);
    void setUri(std::string &uri);
    void setQuery(std::vector<std::pair<std::string, std::string> > &query);
    void setVersion(std::pair<short, short> version);

    const short getMethod() const;
    const std::string &getUri() const;
    const std::vector<std::pair<std::string, std::string> > &getQuery() const;
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

    char _buf[BUF_SIZE];
    char _status;
    size_t _contentLength;

    std::string _remain;
    std::vector<std::string> _lineV;

    HttpRequest _httpRequest;
    HttpRequest _cycleHttpRequest;

    void _inputEOF();
    void _inputStart();

    void _inputRequestLine();
    std::string _decodeUrl(std::string &str);
    void _parseRequestLine();

    void _inputHeaderField();
    void _parseQuery(RequestLine &requestLine, std::string &query);
    void _parseHeaderField();

    void _extractContentLength(int contentLengthCount);
    void _inputMessageBody();
    void _inputDefaultBody();
    void _inputChunkedBody();

    void _pushRequest(std::queue<HttpRequest> &httpRequestQ);

    std::vector<std::string> _splitByComma(std::string &str);
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

    const HttpRequest& getHttpRequest() const;
    void setHttpRequest(const HttpRequest& httpRequest);

    bool closed() const;
};

#endif
