#ifndef CGI_REQUEST_MODULE_HPP
# define CGI_REQUEST_MODULE_HPP

# include <vector>
# include <string>
# include "../../webserv.hpp"
# include "../cycle/Cycle.hpp"
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

	std::vector<std::string>& getMetaVariables();
	std::string& getMessageBody();

};

class CgiRequestHandler
{
private:
	CgiRequest _cgiRequest;
	size_t _pos;
	bool _eof;

	void _setMetaVariables(Cycle* cycle, HttpRequest& httpRequest);
	char** _makeArgv();
	char** _makeEnvp();
	void _parentProcess(int* servToCgi, int* cgiToServ);
	void _childProcess(int* servToCgi, int* cgiToServ);

public:
	CgiRequestHandler();
	CgiRequestHandler& operator=(const CgiRequestHandler& cgiRequestHandler);

	void makeCgiRequest(Cycle* cycle, HttpRequest& httpRequest);
	void sendCgiRequest(const struct kevent& kev);
	void callCgiScript(Cycle* cycle);

	bool eof() const;

    void reset();

};

#endif
