#ifndef CONFIG_INFO_HPP
#define CONFIG_INFO_HPP

#include <netinet/in.h>
#include "../../config/Config.hpp"
#include "../modules/HttpRequest.hpp"

#define DEFAULT_TIMEOUT 10L
#define DEFAULT_CGI_ROOT "var/script/"
#define DEFAULT_MAX_BODY_SIZE 1000000000L

typedef std::map<std::string, std::vector<std::string> > t_directives;

class ConfigInfo
{
private:
    static const std::string DEFAULT_INDEX; // index.html
    static const std::string DEFAULT_ROOT;    // var/www/html
    static const std::map<std::string, std::string> DEFAULT_PAGE;

    std::string _root;
    std::string _cgiPath;    // cgi인지를 판별하게 만드는 경로가 저장된다.
    std::string _path;       // root + uri, 만약 CGI request로 넘어간다면 PATH_INFO에 넘겨주는 인자로 설정이 된다.
    bool _allowMethods[4]; // GET, HEAD, POST, DELETE
    std::string _index;
    // std::string _errorPage;
    std::map<std::string, std::string> _errorPage;
    bool _autoIndex;
    bool _isRedirect;
    intptr_t _requestTimeout;
    intptr_t _keepaliveTimeout;
    std::pair<std::string, std::string> _redirect; // first: redirection 번호(301 or 302), second: redirect uri
    t_directives _info;

    std::vector<ServerConfig>::iterator findMatchedServer(in_addr_t ip, in_port_t port, std::string serverName);
    LocationConfig &findMatchedLocation(std::string &uri, std::map<std::string, LocationConfig> &locationMap, std::string &path);
    void transferInfo(t_directives &directives);

public:
    enum
    {
        MAKE_CGI_REQUEST = 0,
        MAKE_HTTP_RESPONSE
    };

    static const std::string& getDefaultPage(unsigned short code);

    ConfigInfo();
    ConfigInfo(in_addr_t ip, in_port_t port, std::string serverName, std::string uri); // ip, port을 보고 match되는 config를 찾아서 생성자 호출
    ConfigInfo &operator=(const ConfigInfo &ConfigInfo);
    ~ConfigInfo();

    void initConfigInfo(in_addr_t ip, in_port_t port, std::string serverName, std::string uri);

    // test
    void printConfigInfo();
    std::string getPrintableConfigInfo();

    // getter
    std::string getRoot() const;
    std::string getCgiPath() const;
    std::string getPath() const;
    bool getAllowMethods(int index) const;
    std::string getIndex() const;
    std::string getErrorPage(std::string key) const;
    std::string getServerName() const;
    bool getAutoIndex() const;
    t_directives getInfo() const;
    bool getIsRedirect() const;
    intptr_t getRequestTimeout() const;
    intptr_t getKeepaliveTimeout() const;
    std::pair<std::string, std::string> getRedirect() const;
    short requestType(HttpRequest& httpRequest) const;

    void setDefaultErrorPage(unsigned short code);
};

#endif
