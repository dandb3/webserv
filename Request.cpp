#include <cstring>
#include "parser.hpp"
#include "Request.hpp"
#include "Exception.hpp"

extern bool is_origin_form(const std::string& str);
extern bool is_absolute_form(const std::string& str);
extern bool is_authority_form(const std::string& str);
extern bool is_asterisk_form(const std::string& str);

/* ---------------- static functions ---------------- */

static
std::string extract_val(const std::string& line, size_t sep)
{
    size_t val_start = sep, val_end;

    ++val_start;
    while (is_WS(line[val_start]))
        ++val_start;
    val_end = line.size() - 1;
    while (is_WS(line[val_end]))
        --val_end;
    return line.substr(val_start, val_end + 1);
}

static
void check_key(const std::string& key)
{
    if (key.empty())
        throw EParse();
    for (size_t i = 0; i < key.size(); ++i)
        if (!is_tchar(key[i]))
            throw EParse();
}

static
void check_val(const std::string& val)
{
    size_t cur = 0;

    while (cur < val.size()) {
        if (!is_VCHAR(val[cur]))
            throw EParse();
        ++cur;
        if (is_size_fit(val, cur, 2) && is_WS(val[cur]) \
            && is_VCHAR(val[cur + 1]))
            cur += 2;
    }
}

/* ---------------- class Request ---------------- */

void Request::_insertMethod(const std::string& method)
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

void Request::_insertUri(const std::string& uri)
{
    
}

void Request::_insertProtocol(const std::string& protocol)
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

void Request::_insertStart(const std::string& start)
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


void Request::_insertHeader(const std::string& header)
{
    std::string line, key, val;
    size_t lineStart = 0, lineEnd, sep;

    while ((lineEnd = header.find(CRLF, lineStart)) != std::string::npos) {
        line = header.substr(lineStart, lineEnd);
        if ((sep = line.find(':')) == std::string::npos)
            throw EParse();
        key = line.substr(0, sep);
        val = extract_val(line, sep);
        check_key(key);
        check_val(val);
        this->_hdrField[key].push_back(val);
        lineStart = lineEnd;
    }
}

void Request::_insertBody(const std::string& body)
{
    this->_msgBody = body;
}

void Request::parse(const std::string& inMsg)
{
    size_t first, second;

    if ((first = inMsg.find(CRLF)) == std::string::npos)
        throw EParse();
    if ((second = inMsg.find(CRLF CRLF, first)) == std::string::npos)
        throw EParse();
    this->_insertStart(inMsg.substr(0, first));
    this->_insertHeader(inMsg.substr(first + 2, second + 2));
    this->_insertBody(inMsg.substr(second + 4));
}
