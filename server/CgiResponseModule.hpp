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

public:
    void recvCgiResponse(int fd, size_t size);
    void makeCgiResponse();

    const CgiResponse& getCgiResponse() const;

};

#endif
