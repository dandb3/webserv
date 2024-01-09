#ifndef HTTP_RESPONSE_MODULE_HPP
# define HTTP_RESPONSE_MODULE_HPP

# include <string>
# include <map>
# include <utility>
# include "cgi_response.hpp"
# include "HttpRequestModule.hpp"
# include "NetConfig.hpp"

class StatusLine
{
private:
    std::pair<short, short> _version;
    short _code;
    std::string _text;

public:
    StatusLine& operator=(const StatusLine& ref);

    void setVersion(std::pair<short, short> &version);
    void setCode(short code);
    void setText(std::string &text);
};

class HttpResponse
{
private:
	StatusLine _statusLine;
	std::string _messageBody;
	std::multimap<std::string, std::string> _headerFields;

public:
	HttpResponse(int fd, const cgi_response& cres);

	int getStatus() const;
	void setStatus(StatusLine &statusLine);
};

class HttpResponseHandler
{
private:
	std::string _response;
	size_t _pos;
	char _status;

	HttpResponse _httpResponse;

	void _makeStatusLine();
	void _makeHeaderField();
	void _makeMessageBody();

public:
	enum
	{
		RES_IDLE,
		RES_PROCESSING,
		RES_READY,
	};
	HttpResponseHandler();

	void makeHttpResponse(HttpRequest &httpRequest, NetConfig &netConfig);
	void sendHttpResponse(size_t size);
};

#endif
