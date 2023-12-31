#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <vector>
#include <string>
#include "parser.hpp"

class Request
{
private:
    enum method
    {
        GET,
        HEAD,
        POST,
        DELETE
    };

    int _method; // only GET, HEAD, POST, DELETE are allowed
    std::string _scheme;
    std::string _authority;
    std::string _path;
    std::string _query;
    std::pair<int, int> _protocol; // x.x form
    std::map<std::string, std::vector<std::string>> _header_field;
    std::string _msg_body;

    void _insert_method(const std::string &method);
    void _insert_uri(const std::string &uri);
    void _insert_protocol(const std::string &protocol);

    void _insert_start(const std::string &start);
    void _insert_header(const std::string &header);
    void _insert_body(const std::string &body);

public:
    void parse(const std::string &inMsg);
};

#endif
