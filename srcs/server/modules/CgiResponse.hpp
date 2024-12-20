#ifndef CGI_RESPONSE_HPP
#define CGI_RESPONSE_HPP

#include <string>
#include <vector>
#include "../parse/parse.hpp"

class CgiResponse
{
public:
	enum
	{
		DOCUMENT_RES = 0,
		LOCAL_REDIR_RES,
		CLIENT_REDIR_RES,
		CLIENT_REDIRDOC_RES,
        CGI_RESPONSE_ERROR
	};

private:
    unsigned short _statusCode;
    std::vector<pair_t> _headerFields;
    std::string _messageBody;
    char _type;

public:
    CgiResponse();
    CgiResponse(unsigned short code);
    CgiResponse(const std::string& rawCgiResponse);
    CgiResponse& operator=(const CgiResponse& cgiResponse);

    void setStatusCode(unsigned short statusCode);
    void addHeaderField(const pair_t& p);
    void setMessageBody(const std::string& messageBody);
    void setType(char type);

    unsigned short getStatusCode() const;
    std::vector<pair_t>& getHeaderFields();
    std::string& getMessageBody();
    char getType() const;

};

#endif