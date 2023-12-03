#ifndef FD_INFO_HPP
# define FD_INFO_HPP

# include <string>
# include "config.hpp"

# define BUF_SIZE 1023

class http
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

class fd_info
{
private:
    http* _http;
    int _type;

public:
    fd_info();
    fd_info(int type, http* http);

    inline int get_type() const;

};

inline int fd_info::get_type() const
{
    return _type;
}

#endif
