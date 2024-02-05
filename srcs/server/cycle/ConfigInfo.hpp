#ifndef CONFIG_INFO_HPP
#define CONFIG_INFO_HPP

#include <netinet/in.h>
#include "../../config/Config.hpp"
typedef std::map<std::string, std::vector<std::string> > t_directives;

class ConfigInfo
{
private:
	static const std::string DEFAULT_INDEX; // index.html
	static const std::string DEFAULT_ROOT;	// /var/www/html
	static const std::map<std::string, std::string> DEFAULT_PAGE;

	std::string _root;
	std::string _path;	   // root + uri
	bool _allowMethods[4]; // GET, HEAD, POST, DELETE
	std::string _index;
	// std::string _errorPage;
	std::map<std::string, std::string> _errorPage;
	bool _autoIndex;
	bool _isRedirect;
	std::pair<std::string, std::string> _redirect; // first: redirection 번호(301 or 302), second: redirect uri
	t_directives _info;

	std::vector<ServerConfig>::iterator findMatchedServer(in_addr_t ip, in_port_t port, std::string serverName);
	LocationConfig &findMatchedLocation(std::string &uri, std::map<std::string, LocationConfig> &locationMap, std::string &path);
	void transferInfo(t_directives &directives);

public:
	static const std::string& getDefaultErrorPage();

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
	std::string getPath() const;
	bool getAllowMethods(int index) const;
	std::string getIndex() const;
	std::string getErrorPage(std::string key) const;
	bool getAutoIndex() const;
	t_directives getInfo() const;
	bool getIsRedirect() const;
	std::pair<std::string, std::string> getRedirect() const;

  void setDefaultErrorPage();
 
};

#endif
