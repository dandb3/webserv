#ifndef CGI_RESPONSE_MODULE_HPP
# define CGI_RESPONSE_MODULE_HPP

# include <map>
# include <string>
# include <sys/event.h>
# include "../../webserv.hpp"
# include "../parse/CgiResponseParser.hpp"
# include "../cycle/ICycle.hpp"
# include "CgiResponse.hpp"

#define BUF_SIZE 102400UL

class CgiResponseHandler
{
private:
    CgiResponse _cgiResponse;
    std::string _rawCgiResponse;
    bool _eof;

public:
    CgiResponseHandler();
    CgiResponseHandler& operator=(const CgiResponseHandler& cgiResponseHandler);

    void recvCgiResponse(const struct kevent& kev);
    void makeCgiResponse();

    CgiResponse& getCgiResponse();
    char getResponseType() const;
    bool eof() const;

    void reset();

};

#endif
