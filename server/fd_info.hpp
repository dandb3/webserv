#ifndef FD_INFO_HPP
# define FD_INFO_HPP

# include <string>

class cgi
{

};

class http
{
private:
    cgi* _cgi;
    std::string _payload;
    size_t _wpos;
    int _http_sock;
    int _cgi_read;
    int _cgi_write;

public:

};

class fd_info
{
private:
    http* _http;
    std::string _ip;
    int _type;
    short _port;

public:

};

#endif
