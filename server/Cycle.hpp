#ifndef CYCLE_HPP
# define CYCLE_HPP

# include "NetConfig.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "CgiRequest.hpp"
# include "CgiResponse.hpp"

class Cycle
{
private:
    NetConfig _netConfig;
    HttpRequest* _httpRequest;
    HttpResponse* _httpResponse;
    CgiRequest* _cgiRequest;
    CgiResponse* _cgiResponse;

public:

};

#endif
