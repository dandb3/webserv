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
#include "../cycle/ConfigInfo.hpp"
#include "../../utils/utils.hpp"

#define CRLF "\r\n"

class StatusLine
{
public:
	std::pair<short, short> version;
	short code;
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

	void _setAllow();
	void _setConnection(bool disConnected);
	void _setContentLength();
	void _setContentType();
	void _setDate();
	void _setLastModified(const char *path);
	void _setLocation(std::string &location);

	void _makeStatusLine(StatusLine &statusLine, short code);
	void _makeHeaderFields(ConfigInfo &configInfo);

	void _makeGETResponse(HttpRequest &httpRequest, ConfigInfo &configInfo, bool isGET);
	void _makePOSTResponse(HttpRequest &httpRequest, ConfigInfo &configInfo);
	void _makeDELETEResponse(HttpRequest &httpRequest, ConfigInfo &configInfo);

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
	void makeHttpErrorResponse(short code);	
	void sendHttpResponse(int fd, size_t size);
};

#endif
