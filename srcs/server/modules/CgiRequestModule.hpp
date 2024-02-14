#ifndef CGI_REQUEST_MODULE_HPP
# define CGI_REQUEST_MODULE_HPP

# include <vector>
# include <string>
# include <sys/event.h>
# include "../../webserv.hpp"
# include "../cycle/ICycle.hpp"
# include "HttpRequestModule.hpp"
# include "CgiRequest.hpp"

# define CGI_PATH "../../../var/script/cat"

class Cycle;

class CgiRequestHandler
{
private:
	CgiRequest _cgiRequest;
	size_t _pos;
	bool _eof;

	void _setMetaVariables(ICycle* cycle, HttpRequest& httpRequest);
	char** _makeArgv(const std::string& cgiPath);
	char** _makeEnvp();
	void _parentProcess(int* servToCgi, int* cgiToServ);
	void _childProcess(int* servToCgi, int* cgiToServ, const std::string& cgiPath);

public:
	CgiRequestHandler();
	CgiRequestHandler& operator=(const CgiRequestHandler& cgiRequestHandler);

	void makeCgiRequest(ICycle* cycle, HttpRequest& httpRequest);
	void sendCgiRequest(const struct kevent& kev);
	void callCgiScript(ICycle* cycle);

	bool eof() const;

    void reset();

};

#endif
