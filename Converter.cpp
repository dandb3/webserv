#include "Converter.hpp"
#include <unistd.h>

/* ---------------- exceptions ---------------- */
const char* Converter::EInput::what() const throw()
{
    return "input error";
}

const char* Converter::EParse::what() const throw()
{
    return "parse error";
}

/* ---------------- Converter ---------------- */
char Converter::_iBuf[BUFSIZE + 1];

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

    this->_inMsg = conv._inMsg;
    this->_outMsg = conv._outMsg;
    this->_eof = conv._eof;
    return *this;
}

Converter::~Converter()
{}

bool Converter::eof() const
{
    return this->_eof;
}

void Converter::add(int readfd)
{
    ssize_t readlen;

    readlen = read(readfd, this->_iBuf, BUFSIZE);
    if (readlen == -1)
        throw Converter::EInput();
    else if (readlen > 0) {
        this->_iBuf[readlen] = '\0';
        this->_inMsg.append(this->_iBuf);
    }
    if (readlen < BUFSIZE) {
        this->_makeResponse();
        this->_eof = true;
    }
}

void Converter::response(int writefd) const
{
    write(writefd, this->_outMsg.c_str(), this->_outMsg.size());
}
