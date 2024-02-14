#ifndef HTTP_RESPONSE_MODULE_HPP
#define HTTP_RESPONSE_MODULE_HPP

#include <string>
#include <utility>
#include <sstream>
#include <sys/stat.h>
#include <ctime>
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>
#include "CgiResponseModule.hpp"
#include "HttpRequestModule.hpp"
#include "../cycle/ICycle.hpp"
#include "../../utils/utils.hpp"
#include "HttpResponse.hpp"

#define CRLF "\r\n"

class CgiResponse;

class HttpResponseHandler
{
private:
    std::string _response;
    size_t _pos;
    char _status;

    HttpResponse _httpResponse;

    void _setAllow(ConfigInfo& configInfo);
    void _setConnection(ICycle* cycle);
    void _setContentLength();
    void _setContentLength(off_t size);
    void _setContentType(bool isPath, const std::string& str);
    void _setDate();
    void _setLastModified(const char *path);
    void _makeDirectoryListing(const std::string& path);

    void _makeStatusLine();
    void _makeHeaderFields(ICycle* cycle);

    void _makeGETResponse(ICycle* cycle);
    void _makeHEADResponse(ICycle* cycle);
    void _makePOSTResponse(ICycle* cycle, HttpRequest &httpRequest);
    void _makeDELETEResponse(ICycle* cycle);

    void _statusLineToString(std::stringstream &responseStream);
    void _headerFieldsToString(std::stringstream &responseStream);
    void _httpResponseToString();

public:
    enum
    {
        GET,
        HEAD,
        POST,
        DELETE
    };
    enum
    {
        RES_IDLE,
        RES_BUSY,
        RES_FINISH
    };

    HttpResponseHandler();

    void makeHttpResponse(ICycle* cycle, HttpRequest &httpRequest);
    void makeHttpResponse(ICycle* cycle, CgiResponse &cgiResponse);
    void makeErrorHttpResponse(ICycle* cycle);
    void makeHttpResponseFinal(ICycle* cycle);

    void sendHttpResponse(int fd, size_t size);

    void setStatus(char status);

    char getStatus() const;
    HttpResponse& getHttpResponse();

    bool isErrorCode(unsigned short code);
    void reset();

};

#endif
