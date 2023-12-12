#ifndef FD_INFO_HPP
# define FD_INFO_HPP

# include <string>
# include "config.hpp"

# define BUF_SIZE 1024

class info
{
public:
    info(int sockfd);

    std::string payload;
    size_t wpos;
    int http_sock;
    int cgi_read;
    int cgi_write;
    struct cgi cgi_s;
};

class fd_info
{
public:
    enum
    {
        SERV_LISTEN,
        SERV_HTTP_REQ,
        SERV_HTTP_RES,
        SERV_CGI_REQ,
        SERV_CGI_RES,
        SERV_DEFAULT,
        SERV_ERROR
    };

private:
    static char input_buf[BUF_SIZE + 1];

    char _type;
    struct info* _info;

public:
    fd_info(int type, struct info* info);

    inline int get_type() const;
    inline struct info& get_info() const;

};

inline int fd_info::get_type() const
{
    return _type;
}

inline struct info& fd_info::get_info() const
{
    return _info;
}

#endif
