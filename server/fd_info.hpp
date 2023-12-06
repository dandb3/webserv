#ifndef FD_INFO_HPP
# define FD_INFO_HPP

# include <string>
# include "config.hpp"

# define BUF_SIZE 1023

class fd_info
{
private:
    static char _input_buf[BUF_SIZE + 1];

    std::string _payload;
    size_t _wpos;
    int _http_sock;
    int _cgi_read;
    int _cgi_write;
    struct cgi _cgi_params;

public:

};

#endif
