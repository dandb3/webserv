#ifndef CGI_REQUEST_MODULE_HPP
# define CGI_REQUEST_MODULE_HPP

# include <vector>
# include <string>
# include "webserv.hpp"
# include "HttpRequestHandler.hpp"

# define CGI_PATH "undefined"

/**
 * HttpRequest -> CgiRequest로 바꾸는 부분 미구현
*/

class CgiRequest
{
private:
	std::vector<std::string> _metaVariables;
	std::string _messageBody;

public:
	CgiRequest();
	CgiRequest& operator=(const CgiRequest& cgiRequest);

	const std::vector<std::string>& getMetaVariables() const;
	const std::string& getMessageBody() const;

};

class CgiRequestHandler
{
private:
	CgiRequest _cgiRequest;
	size_t _pos;

	char** _makeArgv() const;
	char** _makeEnvp() const;
	void _parentProcess(int* servToCgi, int* cgiToServ) const;
	void _childProcess(int* servToCgi, int* cgiToServ) const;

public:
	void makeCgiRequest(HttpRequest& httpRequest);
	/**
	 * pipe(), fork()
	 * addEvent()를 내부적으로 해주는게 좋을 것 같긴한데
	 * 그러면 handler 클래스의 레퍼런스를 받아와야됨..
	 * -> 구조가 꼬인다!
	*/
	void callCgiScript(int& cgiSendFd, int& cgiRecvFd) const;
	void sendCgiRequest(int fd, size_t size);

};

#endif
