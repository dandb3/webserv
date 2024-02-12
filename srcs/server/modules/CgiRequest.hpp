#ifndef CGI_REQUEST_HPP
#define CGI_REQUEST_HPP

#include <string>
#include <vector>

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

#endif
