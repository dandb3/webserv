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

    static char _buf[BUF_SIZE + 1];
    int _fd;
    char _status;

    std::string _remain;
    std::vector<std::string> _line_v;
    std::queue<http_request> _request_q;

    void _input_start();

    void _input_request_line();
    void _parse_request_line();

    void _input_header_field();
    void _parse_header_field();

    void _input_message_body();

    void _push_request();
    void _push_err_request();

public:
    http_request_parser(int fd);

    void recv_request(size_t size);
    void parse_request(bool eof);

    inline bool closed() const;

};

inline bool http_request_parser::closed() const
{
    return (_status == INPUT_CLOSED);
}

#endif
