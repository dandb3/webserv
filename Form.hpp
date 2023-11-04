#ifndef FORM_HPP
# define FORM_HPP

# include <map>
# include <vector>
# include <string>
# include "parser.hpp"

class AForm
{
protected:
    std::map<std::string, std::vector<std::string> > _hdrField;
    std::string _msgBody;

    virtual void _insertStart(const std::string& start) = 0;
    void _insertHeader(const std::string& header);
    void _insertBody(const std::string& body);

public:
    void parse(const std::string& inMsg);

};

class RequestForm : public AForm
{
private:
	enum
	{
		GET,
		POST,
		DELETE
	};

private:
    int _method;				// only GET, POST, DELETE are allowed
    std::string _uri;
    std::string _protocol;		// x.x form

	void _insertMethod(const std::string& method);
	void _insertUri(const std::string& uri);
	void _insertProtocol(const std::string& protocol);

protected:
    virtual void _insertStart(const std::string& start);

};

class ResponseForm : public AForm
{
private:
    int _statcode;
    std::string _msg;
    std::string _protocol;

	void _insertStatcode(const std::string& statcode);
	void _insertMsg(const std::string& msg);
	void _insertProtocol(const std::string& protocol);

protected:
    virtual void _insertStart(const std::string& start);

};

#endif
