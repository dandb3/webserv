#include <unistd.h>
#include "http_request.hpp"

void http_request::_read_line()
{
    _remain += 
}

void http_request::_read_body()
{

}

void http_request::read_buf()
{
    if ((_input_size = read(_fd, _input_buf, BUF_SIZE)) == FAILURE)
        throw err_syscall();
    _input_pos = 0;
    _input_buf[_input_size] = '\0';
    std::string buf(_input_buf);
    
}
