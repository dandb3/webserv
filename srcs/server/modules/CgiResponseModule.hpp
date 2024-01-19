#ifndef CGI_RESPONSE_MODULE_HPP
# define CGI_RESPONSE_MODULE_HPP

# include <map>
# include <string>
# include "../parse/CgiResponseParser.hpp"

# define BUF_SIZE 1024

class CgiResponse
{
private:
    std::map<std::string, std::string> _headerFields;
    std::string _messageBody;

public:
    CgiResponse();
    CgiResponse(const std::string& rawCgiResponse);
    CgiResponse& operator=(const CgiResponse& cgiResponse);

    void addHeaderField(const pair_t& p);
    void setMessageBody(const std::string& messageBody);

    const std::map<std::string, std::string>& getHeaderFields() const;
    const std::string& getMessageBody() const;

};

class CgiResponseHandler
{
public:
	enum
	{
		DOCUMENT_RES = 0,
		LOCAL_REDIR_RES,
		CLIENT_REDIR_RES,
		CLIENT_REDIR_DOC_RES,
        ERROR
	};

private:
    static char _buf[BUF_SIZE];

    CgiResponse _cgiResponse;
    std::string _rawCgiResponse;
	char _type;
    bool _eof;

public:
    CgiResponseHandler();
    CgiResponseHandler& operator=(const CgiResponseHandler& cgiResponseHandler);

    void recvCgiResponse(struct kevent& kev);
    void makeCgiResponse();

    const CgiResponse& getCgiResponse() const;
    bool eof() const;

};

#endif
