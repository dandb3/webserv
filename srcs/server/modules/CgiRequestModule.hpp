#ifndef CGI_REQUEST_MODULE_HPP
# define CGI_REQUEST_MODULE_HPP

# include <vector>
# include <string>
# include "../../webserv.hpp"
# include "HttpRequestModule.hpp"

# define CGI_PATH "undefined"

class CgiRequest
{
private:
	std::vector<std::string> _metaVariables;
	std::string _messageBody;

public:
	CgiRequest();
	CgiRequest& operator=(const CgiRequest& cgiRequest);

	void addMetaVariable(const std::string& key, const std::string& value);
	void setMessageBody(const std::string& messageBody);

	const std::vector<std::string>& getMetaVariables() const;
	const std::string& getMessageBody() const;

};

class CgiRequestHandler
{
private:
	CgiRequest _cgiRequest;
	size_t _pos;
	bool _eof;

	void _setMetaVariables(HttpRequest& httpRequest);
	char** _makeArgv() const;
	char** _makeEnvp() const;
	void _parentProcess(int* servToCgi, int* cgiToServ) const;
	void _childProcess(int* servToCgi, int* cgiToServ) const;

public:
	CgiRequestHandler();
	CgiRequestHandler& operator=(const CgiRequestHandler& cgiRequestHandler);

	void makeCgiRequest(HttpRequest& httpRequest);
	void sendCgiRequest(const struct kevent& kev);
	void callCgiScript(int& cgiSendFd, int& cgiRecvFd) const;

	bool eof() const;

};

#endif
