#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <string>
# include <map>
# include "webserv.hpp"

# define BUF_SIZE 1024

class http_request
{
private:
//  used for input
    int _fd;
    char _input_buf[BUF_SIZE + 1];
    int _input_pos;
    int _input_size;
    std::string _remain;
    int _content_length;

//  main members of http_request
    std::string _request_line;
    std::multimap<std::string, std::string> _header_fields;
    std::string _message_body;

    void _read_line();
    void _read_body();

public:
    http_request(int fd);

    void read_buf();

};

#endif
