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

    void _setConnection(Cycle* cycle);
    void _setContentLength();
    void _setContentType(Cycle* cycle, const std::string& path);
    void _setDate();
    void _setLastModified(const char *path);

    void _makeStatusLine();
    void _makeHeaderFields(Cycle* cycle);

    void _makeGETResponse(Cycle* cycle, HttpRequest &httpRequest);
    void _makeHEADResponse(Cycle* cycle, HttpRequest &httpRequest);
    void _makePOSTResponse(Cycle* cycle, HttpRequest &httpRequest);
    void _makeDELETEResponse(Cycle* cycle, HttpRequest &httpRequest);

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

    void makeHttpResponse(Cycle* cycle, HttpRequest &httpRequest);
    void makeHttpResponse(Cycle* cycle, const CgiResponse &cgiResponse);
    void makeHttpErrorResponse(Cycle* cycle);
    void makeHttpResponseFinal();

    void sendHttpResponse(int fd, size_t size);

    void setStatus(char status);

    char getStatus() const;
    HttpResponse& getHttpResponse();

    bool isErrorCode(unsigned short code);

};

#endif
