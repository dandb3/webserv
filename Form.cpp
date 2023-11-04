#include <cstring>
#include "Form.hpp"
#include "Exception.hpp"

/* ---------------------- AForm ---------------------- */
void AForm::_insertHeader(const std::string& header)
{

}

void AForm::_insertBody(const std::string& body)
{
    
}

void AForm::parse(const std::string& inMsg)
{
    size_t first, second;

    if ((first = inMsg.find(CRLF)) == std::string::npos)
        throw EParse();
    if ((second = inMsg.find(CRLF CRLF, first)) == std::string::npos)
        throw EParse();
    this->_insertStart(inMsg.substr(0, first));
    this->_insertHeader(inMsg.substr(first + 2, second));
    this->_insertBody(inMsg.substr(second + 4));
}

/* ---------------------- RequestForm ---------------------- */

void RequestForm::_insertMethod(const std::string& method)
{
    if (method == "GET")
        this->_method = GET;
    else if (method == "POST")
        this->_method = POST;
    else if (method == "DELETE")
        this->_method = DELETE;
    else
        throw EParse();
}

void RequestForm::_insertUri(const std::string& uri)
{

}

void RequestForm::_insertProtocol(const std::string& protocol)
{
    size_t pos;

    if ((pos = protocol.find('/')) == std::string::npos)
        throw EParse();
    if (protocol.substr(0, pos) != HTTP_NAME)
        throw EParse();
    this->_protocol = protocol.substr(pos + 1);
    if (this->_protocol.size() != 3 || !isdigit(this->_protocol[0]) \
        || this->_protocol[1] != '.' || !isdigit(this->_protocol[2]))
        throw EParse();
}

void RequestForm::_insertStart(const std::string& start)
{
    size_t first, second;

    if ((first = start.find(SP)) == std::string::npos)
        throw EParse();
    if ((second = start.find(SP, first + 1)) == std::string::npos)
        throw EParse();
    this->_insertMethod(start.substr(0, first));
    this->_insertUri(start.substr(first + 1, second));
    this->_insertProtocol(start.substr(second + 1));
}

/* ---------------------- ResponseForm ---------------------- */

void ResponseForm::_insertStatcode(const std::string& statcode)
{

}

void ResponseForm::_insertMsg(const std::string& msg)
{

}

void ResponseForm::_insertProtocol(const std::string& protocol)
{

}

void ResponseForm::_insertStart(const std::string& start)
{
    size_t first, second;

    if ((first = start.find(SP)) == std::string::npos)
        throw EParse();
    if ((second = start.find(SP, first)) == std::string::npos)
        throw EParse();
    this->_insertStatcode(start.substr(0, first));
    this->_insertMsg(start.substr(first, second));
    this->_insertProtocol(start.substr(second));
}
