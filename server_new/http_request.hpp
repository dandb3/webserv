#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <string>
# include <map>

# define BUF_SIZE 1024

class http_request
{
private:
    static char _input_buffer[BUF_SIZE + 1];
    std::string 

    std::string _request_line;
    std::multimap<std::string, std::string> _header_fields;
    std::string _message_body;

public:

};

#endif
