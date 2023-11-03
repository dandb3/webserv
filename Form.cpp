#include "Form.hpp"
#include "Exception.hpp"

void AForm::_insertHeader(const std::string& header)
{

}

void AForm::_insertBody(const std::string& body)
{
    
}

void AForm::parse(const std::string& inMsg)
{
    size_t first, second;

    if ((first = inMsg.find("\r\n")) == std::string::npos)
        throw EParse();
    this->_insertStart(inMsg.substr(0, first));
    if ((second = inMsg.find("\r\n\r\n", first)) == std::string::npos)
        throw EParse();
    this->_insertHeader(inMsg.substr(first, second));
    this->_insertBody(inMsg.substr(second));
}
