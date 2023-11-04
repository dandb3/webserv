#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <map>
# include <vector>
# include <string>
# include "parser.hpp"

class Request
{
private:
	enum method
	{
		GET,
		POST,
		DELETE
	};

    int _method;				// only GET, POST, DELETE are allowed
    std::string _uri;
    std::string _protocol;		// x.x form
    std::map<std::string, std::vector<std::string> > _hdrField;
    std::string _msgBody;

	void _insertMethod(const std::string& method);
	void _insertUri(const std::string& uri);
	void _insertProtocol(const std::string& protocol);

    void _insertStart(const std::string& start);
    void _insertHeader(const std::string& header);
    void _insertBody(const std::string& body);

public:
    void parse(const std::string& inMsg);

};

#endif
