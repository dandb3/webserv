#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <string>
# include <vector>

struct cgi
{

};

struct location
{
    std::vector<struct location> _locations;
    std::string error_page;
    std::string index;
    std::string root;
    int client_max_body_size;
    bool autoindex;
    struct cgi cgi_params;
};

class server
{
private:
    std::vector<struct location> _locations;
    std::string _server_name;
    std::string _ip;
    u_short _port;

public:
    inline const std::string& get_ip() const;
    inline u_short get_port() const;

};

inline const std::string& server::get_ip() const
{
    return _ip;
}

inline u_short server::get_port() const
{
    return _port;
}

class config
{
private:
    std::vector<server> _servers;

public:
    inline const std::vector<server>& get_servers() const;

};

inline const std::vector<server>& config::get_servers() const
{
    return _servers;
}

#endif
