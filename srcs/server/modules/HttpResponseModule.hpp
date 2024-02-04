#ifndef HTTP_RESPONSE_MODULE_HPP
#define HTTP_RESPONSE_MODULE_HPP

#include <string>
#include <map>
#include <utility>
#include <sys/stat.h>
#include <ctime>
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>
#include "CgiResponseModule.hpp"
#include "HttpRequestModule.hpp"
#include "../cycle/Cycle.hpp"
#include "../../utils/utils.hpp"

#define CRLF "\r\n"

class StatusLine
{
public:
    std::pair<short, short> version;
    unsigned short code;
    std::string text;

    StatusLine &operator=(const StatusLine &ref);
};

class HttpResponse
{
public:
    StatusLine statusLine;
    std::multimap<std::string, std::string> headerFields;
    std::string messageBody;
    
    // HttpResponse();

    // getter
    // const StatusLine &getStatusLine() const;
    // std::multimap<std::string, std::string> &getHeaderFields();
    // const std::string &getMessageBody() const;

    // // setter
    // void setStatusLine(StatusLine &statusLine);
    // void setHeaderFields(std::multimap<std::string, std::string> &headerFields);
    // void setMessageBody(std::string &messageBody);
};

class HttpResponseHandler
{
private:
    std::string _response;
    size_t _pos;
    char _status;

    HttpResponse _httpResponse;

    bool _isErrorCode(unsigned short code);

    void _setConnection(std::multimap<std::string, std::string> &headerFields);
    void _setContentLength(std::multimap<std::string, std::string> &headerFields);
    void _setContentType(std::multimap<std::string, std::string> &headerFields);
    void _setDate(std::multimap<std::string, std::string> &headerFields);
    void _setLastModified(std::multimap<std::string, std::string> &headerFields, const char *path);

    void _makeStatusLine(StatusLine &statusLine, short code);
    void _makeHeaderFields(std::multimap<std::string, std::string> &headerFields, ConfigInfo &configInfo);

    void _makeGETResponse(HttpRequest &httpRequest, ConfigInfo &configInfo, bool isGET);
    void _makePOSTResponse(HttpRequest &httpRequest, ConfigInfo &configInfo);
    void _makeDELETEResponse(HttpRequest &httpRequest, ConfigInfo &configInfo);

    void _makeHttpErrorResponse(Cycle* cycle);
    void _makeHttpResponseFinal();

    void _statusLineToString();
    void _headerFieldsToString();
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
        RES_READY
    };
    HttpResponseHandler();

    void makeHttpResponse(HttpRequest &httpRequest, ConfigInfo &configInfo);
    void makeHttpResponse(Cycle* cycle, const CgiResponse &cgiResponse);
    void sendHttpResponse(int fd, size_t size);

    void setStatus(char status);

    char getStatus() const;

};

#endif
