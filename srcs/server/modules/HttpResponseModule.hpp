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
private:
	std::pair<short, short> _version;
	short _code;
	std::string _text;

public:
	StatusLine &operator=(const StatusLine &ref);

	// setter
	void setVersion(std::pair<short, short> version);
	void setCode(short code);
	void setText(std::string &text);

	// getter
	const std::pair<short, short> &getVersion() const;
	const short getCode() const;
	const std::string &getText() const;
};

class HttpResponse
{
private:
	StatusLine _statusLine;
	std::multimap<std::string, std::string> _headerFields;
	std::string _messageBody;

public:
	// HttpResponse();

	// getter
	const StatusLine &getStatusLine() const;
	std::multimap<std::string, std::string> &getHeaderFields();
	const std::string &getMessageBody() const;

	// setter
	void setStatusLine(StatusLine &statusLine);
	void setHeaderFields(std::multimap<std::string, std::string> &headerFields);
	void setMessageBody(std::string &messageBody);
};

class HttpResponseHandler
{
private:
	std::string _response;
	size_t _pos;
	char _status;

	HttpResponse _httpResponse;

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
	void makeHttpResponse(const CgiResponse &cgiResponse);
	void sendHttpResponse(int fd, size_t size);

	void setStatus(char status);

	char getStatus() const;

};

#endif
