#include "Exception.hpp"

const char* EInput::what() const throw()
{
    return "input error";
}

const char* EParse::what() const throw()
{
    return "parse error";
}