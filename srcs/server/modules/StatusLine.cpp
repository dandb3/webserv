#include "HttpResponseModule.hpp"

StatusLine& StatusLine::operator=(const StatusLine& ref)
{
    if (this != ref) {
        _version = ref._version;
        _code = ref._code;
        _text = ref._text;
    }
    return *this;
}

void StatusLine::setVersion(std::pair<short, short> &version)
{
    _version = version;
}

void StatusLine::setCode(short code)
{
    _code = code;
}

void StatusLine::setText(std::string &text)
{
    _text = text;
}

const std::pair<short, short>& StatusLine::getVersion() const
{
    return _version;
}

const short StatusLine::getCode() const
{
    return _code;
}

const std::string& StatusLine::getText() const
{
    return _text;
}
