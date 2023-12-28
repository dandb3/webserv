#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <vector>
#include <map>
#include "webserv.hpp"

#define BUF_SIZE 1024

class http_request
{
private:
    class request_line
    {
    private:
        short _method;
        std::string _request_target;
        std::pair<short, short> _version;

    public:
        enum method
        {
            GET,
            HEAD,
            POST,
            DELETE
        };

        request_line(short method, std::string uri, std::pair<short, short> version);
        request_line &operator=(const request_line &ref);

        void http_request::request_line::set_method(short method);
        void http_request::request_line::set_request_target(std::string _request_target);
        void http_request::request_line::set_version(std::pair<short, short> version);

        int get_method() const;
        std::string get_request_target() const;
        std::pair<short, short> get_version() const;
    };

    enum
    {
        INPUT_REQUEST_LINE,
        INPUT_HEADER_FIELD,
        PARSE_REQUEST_LINE,
        PARSE_HEADER_FIELD,
        INPUT_MESSAGE_BODY,
        INPUT_FINISH
    };

    static char _buf[BUF_SIZE + 1];
    int _fd;
    std::string _remain;
    char _status;

    std::vector<std::string> _line_v;
    request_line _request_line;
    std::multimap<std::string, std::string> _header_fields;
    std::string _message_body;

    bool _input_request_line();
    bool _parse_request_line();

    void _input_header_field();
    bool _parse_header_field();

    void _input_message_body();

    void _input_default_body();
    void _input_chunked_body();

public:
    http_request(int fd);

    void read_input();

    inline const http_request::request_line &get_request_line();
    inline const std::multimap<std::string, std::string> &get_header_fields();
    inline const std::string &get_message_body();
};

inline const http_request::request_line &http_request::get_request_line()
{
    return _request_line;
}

inline const std::multimap<std::string, std::string> &http_request::get_header_fields()
{
    return _header_fields;
}

inline const std::string &http_request::get_message_body()
{
    return _message_body;
}

#endif
