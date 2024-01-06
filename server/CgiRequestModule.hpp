#ifndef CGI_REQUEST_MODULE_HPP
# define CGI_REQUEST_MODULE_HPP

# include <vector>
# include "HttpRequestHandler.hpp"

class CgiRequest
{
private:
	std::vector<std::string> _metaVariables;
	std::string _messageBody;
	CgiRequest();

};

class CgiRequestHandler
{
private:
	friend class CgiRequest;

	CgiRequest _cgiRequest;
	size_t _pos;

	char** _makeEnvp() const;
	void _parentProcess();
	void _childProcess();

public:
	void makeCgiRequest(HttpRequest& httpRequest);
	// socketpair(), fork()
	// addEvent()를 내부적으로 해주는게 좋을 것 같긴한데 그러면 handler 클래스의 
	/**
	 * socketpair(), fork()
	 * addEvent()를 내부적으로 해주는게 좋을 것 같긴한데
	 * 그러면 handler 클래스의 레퍼런스를 받아와야됨..
	 * -> 구조가 꼬인다!
	*/
	void callCgiScript() const;
	void sendCgiRequest(int sockfd, size_t size);

};

#endif
