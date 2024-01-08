#ifndef CGI_RESPONSE_MODULE_HPP
# define CGI_RESPONSE_MODULE_HPP

# include <string>

# define BUF_SIZE 1024

class CgiResponse
{
private:
    // header-fields
    // message-body

public:
    CgiResponse();
    CgiResponse(const std::string& rawCgiResponse);
    CgiResponse& operator=(const CgiResponse& cgiResponse);

    // getter of header-fields;
    // getter of message-body;

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

    void recvCgiResponse(struct kevent& kev);
    void makeCgiResponse();

    const CgiResponse& getCgiResponse() const;
    bool eof() const;

};

#endif
