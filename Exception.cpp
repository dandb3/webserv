#include "Exception.hpp"

const char* Err_syscall::what() const throw()
{
    return "syscall error";
}

const char* Err_input::what() const throw()
{
    return "input error";
}

const char* Err_parse::what() const throw()
{
    return "parse error";
}