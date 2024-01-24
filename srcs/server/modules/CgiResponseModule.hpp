#ifndef CGI_RESPONSE_MODULE_HPP
# define CGI_RESPONSE_MODULE_HPP

# include <map>
# include <string>
# include "../parse/CgiResponseParser.hpp"

# define BUF_SIZE 1024

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
    std::string _reasonPhrase;
    std::vector<pair_t> _headerFields;
    std::string _messageBody;
    char _type;

public:
    CgiResponse();
    CgiResponse(const std::string& rawCgiResponse);
    CgiResponse& operator=(const CgiResponse& cgiResponse);

    void setStatusCode(unsigned short statusCode);
    void setReasonPhrase(const std::string& reasonPhrase);
    void addHeaderField(const pair_t& p);
    void setMessageBody(const std::string& messageBody);
    void setType(char type);

    unsigned short getStatusCode() const;
    const std::string& getReasonPhrase() const;
    const std::vector<pair_t>& getHeaderFields() const;
    const std::string& getMessageBody() const;
    char getType() const;

};

class CgiResponseHandler
{
private:
    static char _buf[BUF_SIZE];

    CgiResponse _cgiResponse;
    std::string _rawCgiResponse;
    bool _eof;

public:
    CgiResponseHandler();
    CgiResponseHandler& operator=(const CgiResponseHandler& cgiResponseHandler);

    void recvCgiResponse(const struct kevent& kev);
    void makeCgiResponse();

    const CgiResponse& getCgiResponse() const;
    char getResponseType() const;
    bool eof() const;

};

#endif
