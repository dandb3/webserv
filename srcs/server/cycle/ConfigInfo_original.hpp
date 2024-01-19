#ifndef CONFIG_INFO_HPP
#define CONFIG_INFO_HPP

#include <netinet/in.h>
#include "../../config/Config.hpp"
typedef std::map<std::string, std::vector<std::string> > t_directives;

// int 자료형을 쓸 것인지, string 자료형을 쓸 것인지, 인자로 어떤 자료형을 받을 것인지, network byte order를 따를지 등은 논의 필요.
// 사실 얘는 그냥 public 멤버 변수들로만 구성해도 괜찮을 것 같은데..
class ConfigInfo
{
private:
	static const std::string DEFAULT_INDEX;		 // index.html
	static const std::string DEFAULT_ROOT;		 // /var/www/html
	static const std::string DEFAULT_ERROR_PAGE; // /confTest/error/defaultError.html

	std::string _root;	   // root
	std::string _path;	   // root + uri
	bool _allowMethods[4]; // GET, HEAD, POST, DELETE
	std::string _index;
	std::string _errorPage;
	// std::map<std::string, std::string> _errorPage;
	bool _autoIndex;
	t_directives _info;

	/**
	 * root
	 * error_page
	 * autoindex
	 * index
	 * etc...
	 */

public:
	ConfigInfo();
	ConfigInfo(in_addr_t ip, in_port_t port, std::string uri); // ip, port을 보고 match되는 config를 찾아서 생성자 호출
	ConfigInfo &operator=(const ConfigInfo &ConfigInfo);
	~ConfigInfo();

	void initConfigInfo(in_addr_t ip, in_port_t port, std::string uri);
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
