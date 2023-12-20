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
            INPUT_PARSE,
            INPUT_MESSAGE_BODY,
            INPUT_FINISH,
        };

        static char _buf[BUF_SIZE + 1];
        int _fd;
        std::string _remain;
        size_t body_size;
        char _status;

        std::vector<std::string> _http_request_v;

        void _read_request_line();
        void _read_header_field();
        void _read_message_body();

        void _read_default_body();
        void _read_chunked_body();

    public:
        parser(int fd);

        void read_buf();

    };

    parser _parser;

    std::string _request_line;
    std::multimap<std::string, std::string> _header_fields;
    std::string _message_body;

public:
    http_request(int fd);

    void read_request();
    void parse();

};

#endif
