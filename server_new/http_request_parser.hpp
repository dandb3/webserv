#ifndef HTTP_REQUEST_PARSER_HPP
# define HTTP_REQUEST_PARSER_HPP

# include <string>
# include <vector>
# include <queue>
# include <map>
# include "http_request.hpp"
# include "webserv.hpp"

# define BUF_SIZE 1024

class http_request_parser
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

        void http_request_parser::request_line::set_method(short method);
        void http_request_parser::request_line::set_request_target(std::string _request_target);
        void http_request_parser::request_line::set_version(std::pair<short, short> version);

        int get_method() const;
        std::string get_request_target() const;
        std::pair<short, short> get_version() const;
    };

    enum
    {
        INPUT_READY,
        INPUT_REQUEST_LINE,
        PARSE_REQUEST_LINE,
        INPUT_HEADER_FIELD,
        PARSE_HEADER_FIELD,
        INPUT_MESSAGE_BODY,
        PARSE_FINISHED,
        INPUT_CLOSED,
    };

    static char _buf[BUF_SIZE];
    int _fd;
    char _status;

    std::string _remain;
    std::vector<std::string> _line_v;
    std::queue<http_request> _http_request_q;

    request_line _request_line;
    std::multimap<std::string, std::string> _header_fields;
    std::string _message_body;

    void _input_start();

    void _input_request_line();
    void _parse_request_line();

    void _input_header_field();
    void _parse_header_field();

    void _input_message_body();
    void _input_default_body(int content_length_count, int transfer_encoding_count);
    void _input_chunked_body(int transfer_encoding_count);

    void _push_request();
    void _push_err_request();


public:
    http_request_parser(int fd);

    void recv_request(size_t size);
    void parse_request(bool eof);

    inline std::queue<http_request>& get_queue();
    inline bool closed() const;

};

inline std::queue<http_request>& http_request_parser::get_queue()
{
    return _http_request_q;
}

inline bool http_request_parser::closed() const
{
    return (_status == INPUT_CLOSED);
}

#endif
