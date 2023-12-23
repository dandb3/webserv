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
        enum method
        {
            GET,
            HEAD,
            POST,
            DELETE
        };

        short _method;
        std::string _uri;
        std::string _version;

    public:
        bool parse(std::string line);

        int get_method() const;
        std::string get_uri() const;
        std::string get_version() const;
    };

    class Header
    {
    private:
        std::multimap<std::string, std::string> elements;

    public:
        bool parse();
        std::string get_element(std::string);
    }

    enum {
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

    void _input_request_line();
    void _parse_request_line();

    void _input_header_field();
    void _parse_header_field();

    void _input_message_body();

    void _input_default_body();
    void _input_chunked_body();

public:
    http_request(int fd);

    void read_input();

    inline const std::string &get_request_line();
    inline const std::multimap<std::string, std::string> &get_header_fields();
    inline const std::string &get_message_body();
};

inline const std::string &http_request::get_request_line()
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
