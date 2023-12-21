#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <string>
# include <vector>
# include <map>
# include "webserv.hpp"

# define BUF_SIZE 1024

class http_request
{
private:
    class parser
    {
    private:
        enum
        {
            INPUT_REQUEST_LINE,
            INPUT_HEADER_FIELD,
            PARSE_REQUEST_LINE,
            PARSE_HEADER_FIELD,
            INPUT_MESSAGE_BODY,
            INPUT_FINISH,
        };

        static char _buf[BUF_SIZE + 1];
        int _fd;
        std::string _remain;
        size_t body_size;
        char _status;

        std::vector<std::string> _http_request_v;
        std::string& _request_line;
        std::multimap<std::string, std::string>& _header_fields;
        std::string& _message_body;

        void _input_request_line();
        void _parse_request_line();

        void _input_header_field();
        void _parse_header_field();

        void _input_message_body();

        void _input_default_body();
        void _input_chunked_body();

    public:
        parser(int fd, std::string& r, std::multimap<std::string, std::string>& h, std::string& m);

        void input_parse();

    };

    std::string _request_line;
    std::multimap<std::string, std::string> _header_fields;
    std::string _message_body;
    parser _parser;

public:
    http_request(int fd);

    inline void read_input();

    inline const std::string& get_request_line();
    inline const std::multimap<std::string, std::string>& get_header_fields();
    inline const std::string& get_message_body();

};

inline void http_request::read_input()
{
    _parser.input_parse();
}

inline const std::string& http_request::get_request_line()
{
    return _request_line;
}

inline const std::multimap<std::string, std::string>& http_request::get_header_fields()
{
    return _header_fields;
}

inline const std::string& http_request::get_message_body()
{
    return _message_body;
}

#endif
