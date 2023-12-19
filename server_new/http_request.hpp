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
            INPUT_LINE = 0,
            INPUT_BODY,
        };

        static char _buf[BUF_SIZE + 1];
        int _fd;
        std::string _remain;
        bool _mode;

        std::vector<std::string> _http_request_line_v;

        void _read_line();
        void _read_body();

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
