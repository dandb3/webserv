#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <string>
# include <vector>
# include <queue>
# include <map>
# include "webserv.hpp"

class http_request
{
private:
    class request_line
    {
    private:
        short _method;
        std::string _request_target;
        std::pair<short, short> _version;

    public:
        enum method
        {
            GET,
            HEAD,
            POST,
            DELETE
        };

        request_line(short method, std::string uri, std::pair<short, short> version);
        request_line &operator=(const request_line &ref);

        void http_request::request_line::set_method(short method);
        void http_request::request_line::set_request_target(std::string _request_target);
        void http_request::request_line::set_version(std::pair<short, short> version);

        int get_method() const;
        std::string get_request_target() const;
        std::pair<short, short> get_version() const;
    };

    enum
    {
        INPUT_READY,
        INPUT_REQUEST_LINE,
        PARSE_REQUEST_LINE,
        INPUT_HEADER_FIELD,
        PARSE_HEADER_FIELD,
        INPUT_MESSAGE_BODY,
        PARSE_FINISHED,
        INPUT_CLOSED,
    };

    int _fd;

    request_line _request_line;
    std::multimap<std::string, std::string> _header_fields;
    std::string _message_body;


public:
    http_request(int fd);

    void recv_request(size_t size);
    void parse_request(bool eof);

    inline bool closed() const;

};

inline bool http_request::closed() const
{
    return (_status == INPUT_CLOSED);
}

#endif
