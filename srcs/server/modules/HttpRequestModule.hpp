#ifndef HTTP_REQUEST_MODULE_HPP
#define HTTP_REQUEST_MODULE_HPP

#include <string>
#include <unistd.h>
#include <vector>
#include <queue>
#include <map>
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
    RequestLine _requestLine;
    std::multimap<std::string, std::string> _headerFields;
    std::string _messageBody;

public:
    HttpRequest();
    HttpRequest(RequestLine &requestLine,
                std::multimap<std::string, std::string> &headerFields, std::string &messageBody);

    void setRequestLine(RequestLine &requestLine);
    void setHeaderFields(std::multimap<std::string, std::string> &headerFields);
    void setMessageBody(std::string &messageBody);

    RequestLine &getRequestLine();
    std::multimap<std::string, std::string> &getHeaderFields();
    std::string getMessageBody() const;
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
        INPUT_CLOSED,
    };

    char _buf[BUF_SIZE];
    char _status;
    size_t _contentLength;

    std::string _remain;
    std::vector<std::string> _lineV;

    HttpRequest _httpRequest;

    void _inputStart();

    void _inputRequestLine();
    std::string _decodeUrl(std::string &str);
    bool _parseRequestLine();

    void _inputHeaderField();
    void _parseQuery(RequestLine &requestLine, std::string &query);
    bool _parseHeaderField();

    void _inputMessageBody();
    void _inputDefaultBody(int content_length_count, int transfer_encoding_count);
    void _inputChunkedBody(int transfer_encoding_count);

    void _push_request(std::queue<HttpRequest> &httpRequestQ);
    void _push_err_request(std::queue<HttpRequest> &httpRequestQ);

    std::vector<std::string> _splitByComma(std::string &str);
public:
    enum
    {
        GET,
        HEAD,
        POST,
        DELETE
    };
    HttpRequestHandler();

    void recvHttpRequest(int fd, size_t size);
    void parseHttpRequest(bool eof, std::queue<HttpRequest> &httpRequestQ);

    inline bool closed() const;
};

inline bool HttpRequestHandler::closed() const
{
    return (_status == INPUT_CLOSED);
}

#endif
