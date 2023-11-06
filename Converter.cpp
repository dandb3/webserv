#include <unistd.h>
#include "Request.hpp"
#include "Response.hpp"
#include "Converter.hpp"

char Converter::_input_buf[BUFSIZE + 1];

Converter::Converter()
: _eof(false)
{}

Converter::Converter(const Converter& conv)
: _eof(conv._eof)
{}

Converter& Converter::operator=(const Converter& conv)
{
    if (this == &conv)
        return *this;

    this->_in_msg = conv._in_msg;
    this->_out_msg = conv._out_msg;
    this->_eof = conv._eof;
    return *this;
}

Converter::~Converter()
{}

void Converter::_convert()
{
    Request request;
    Response response;

    parse(this->_in_msg, request);
    makeResponse(request, response);
    makeMessage(response, this->_out_msg);
}

bool Converter::eof() const
{
    return this->_eof;
}

void Converter::add(int readfd)
{
    ssize_t readlen;

    readlen = read(readfd, this->_input_buf, BUFSIZE);
    if (readlen == -1)
        throw EInput();
    else if (readlen > 0) {
        this->_input_buf[readlen] = '\0';
        this->_in_msg.append(this->_input_buf);
    }
    if (readlen < BUFSIZE) {
        this->_convert();
        this->_eof = true;
    }
}

void Converter::response(int writefd) const
{
    write(writefd, this->_out_msg.c_str(), this->_out_msg.size());
}
