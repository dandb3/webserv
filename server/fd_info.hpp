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
    std::string _ip;
    int _type;
    u_short _port;

public:
    fd_info(const std::string& ip, u_short port, int type, http* http);

    inline int get_type() const;
    inline std::string get_ip() const;
    inline u_short get_port() const;

};

inline int fd_info::get_type() const
{
    return this->_type;
}

inline std::string fd_info::get_ip() const
{
    return this->_ip;
}

inline u_short fd_info::get_port() const
{
    return this->_port;
}

#endif
