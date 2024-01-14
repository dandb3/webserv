#ifndef HTTP_REQUEST_MODULE_HPP
# define HTTP_REQUEST_MODULE_HPP

# include <string>
# include <vector>
# include <queue>
# include <map>
# include "../../webserv.hpp"

# define BUF_SIZE 1024

class RequestLine
{
private:
    enum
    {
        GET,
        HEAD,
        POST,
        DELETE
    };
    short _method;
    std::string _requestTarget;
    std::pair<short, short> _version;

public:
    RequestLine& operator=(const RequestLine& ref);

    void setMethod(short method);
    void setRequestTarget(std::string &requestTarget);
    void setVersion(std::pair<short, short> &version);

    const short getMethod() const;
    const std::string &getRequestTarget() const;
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
    HttpRequest(RequestLine &requestLine, \
        std::multimap<std::string, std::string> &headerFields, std::string &messageBody);

    void setRequestLine(RequestLine &requestLine);
    void setHeaderFields(std::multimap<std::string, std::string> &headerFields);
    void setMessageBody(std::string &messageBody);

    const RequestLine &getRequestLine() const;
    const std::multimap<std::string, std::string> &getHeaderFields() const;
    const std::string &getMessageBody() const;

    inline bool closed() const;
};

inline bool HttpRequest::closed() const
{
    return (_status == INPUT_CLOSED);
}

class HttpRequestHandler
{
private:
    enum
    {
        INPUT_READY,
        INPUT_REQUEST_LINE,
        INPUT_HEADER_FIELD,
        INPUT_MESSAGE_BODY,
        PARSE_FINISHED,
        INPUT_CLOSED,
    };

    static char _buf[BUF_SIZE];
    char _status;

    std::string _remain;
    std::vector<std::string> _lineV;

    HttpRequest _httpRequest;

    void _inputStart();

    void _inputRequestLine();
    void _parseRequestLine();

    void _inputHeaderField();
    void _parseHeaderField();

    void _inputMessageBody();
    void _inputDefaultBody(int content_length_count, int transfer_encoding_count);
    void _inputChunkedBody(int transfer_encoding_count);

    void _push_request();
    void _push_err_request();

public:
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
