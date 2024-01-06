#ifndef HTTP_REQUEST_MODULE_HPP
# define HTTP_REQUEST_MODULE_HPP

# include <string>
# include <vector>
# include <queue>
# include <map>
# include "webserv.hpp"

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
};

class HttpRequest
{
private:
    RequestLine _requestLine;
    std::multimap<std::string, std::string> _headerFields;
    std::string _messageBody;

public:
    HttpRequest(int fd);
    HttpRequest(RequestLine &requestLine, \
        std::multimap<std::string, std::string> &headerFields, std::string &messageBody);

    void setRequestLine(RequestLine &requestLine);
    void setHeaderFields(std::multimap<std::string, std::string> &headerFields);
    void setMessageBody(std::string &messageBody);

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
    int _fd;
    char _status;

    std::string _remain;
    std::vector<std::string> _lineV;
    std::queue<HttpRequest>& _httpRequestQ;

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
    HttpRequestHandler(int fd);

    void recv_request(size_t size);
    void parse_request(bool eof);

    inline std::queue<http_request>& get_queue();
    inline bool closed() const;

};

inline std::queue<http_request>& HttpRequestHandler::get_queue()
{
    return _http_request_q;
}

inline bool HttpRequestHandler::closed() const
{
    return (_status == INPUT_CLOSED);
}


#endif
