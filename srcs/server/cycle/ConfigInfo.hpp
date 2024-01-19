#ifndef ASD
#define ASD

#include <string>

class ConfigInfo
{
private:
	static const std::string DEFAULT_INDEX;		 // index.html
	static const std::string DEFAULT_ROOT;		 // /var/www/html
	static const std::string DEFAULT_ERROR_PAGE; // /confTest/error/defaultError.html

	std::string _root;
	std::string _path;	   // root + uri
	bool _allowMethods[4]; // GET, HEAD, POST, DELETE
	std::string _index;
	std::string _errorPage;
	// std::map<std::string, std::string> _errorPage;
	bool _autoIndex;
	t_directives _info;

	std::vector<ServerConfig>::iterator findMatchedServer(in_addr_t ip, in_port_t port);
	LocationConfig &findMatchedLocation(std::string &uri, std::map<std::string, LocationConfig> &locationMap, std::string &path);
	void transferInfo(t_directives &directives);

public:
	ConfigInfo();
	ConfigInfo(in_addr_t ip, in_port_t port, std::string uri); // ip, port을 보고 match되는 config를 찾아서 생성자 호출
	ConfigInfo &operator=(const ConfigInfo &ConfigInfo);
	~ConfigInfo();

	void initConfigInfo(in_addr_t ip, in_port_t port, std::string uri);

	// test
	void printConfigInfo();
	std::string getPrintableConfigInfo();

	// getter
	std::string getRoot() const;
	std::string getPath() const;
	bool getAllowMethods(int index) const;
	std::string getIndex() const;
	std::string getErrorPage() const;
	bool getAutoIndex() const;
	t_directives getInfo() const;
};

#endif