#ifndef HTTP_RESPONSE_MODULE_HPP
# define HTTP_RESPONSE_MODULE_HPP

# include <string>
# include <map>
# include <utility>
# include "CgiResponseModule.hpp"
# include "HttpRequestModule.hpp"
# include "../cycle/NetConfig.hpp"

class StatusLine
{
private:
    std::pair<short, short> _version;
    short _code;
    std::string _text;

public:
    StatusLine& operator=(const StatusLine& ref);

	// setter
    void setVersion(std::pair<short, short> &version);
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
	const std::multimap<std::string, std::string> &getHeaderFields() const;
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

	void _makeStatusLine(StatusLine &statusLine, short code);
	
	void _makeGETResponse(HttpRequest &httpRequest, NetConfig &netConfig, bool isGET);
	void _makeHEADResponse(HttpRequest &httpRequest, NetConfig &netConfig);
	void _makePUTResponse(HttpRequest &httpRequest, NetConfig &netConfig);
	void _makeDELETEResponse(HttpRequest &httpRequest, NetConfig &netConfig);

public:
	enum
	{
		RES_IDLE,
		RES_PROCESSING,
		RES_READY,
	};
	// HttpResponseHandler();

	void makeHttpResponse(HttpRequest &httpRequest, NetConfig &netConfig);
	void sendHttpResponse(int fd, size_t size);
};

#endif
