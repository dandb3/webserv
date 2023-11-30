#include "Exception.hpp"

const char* err_syscall::what() const throw()
{
    return "syscall error";
}

const char* err_input::what() const throw()
{
    return "input error";
}

const char* err_parse::what() const throw()
{
    return "parse error";
}