#include "parse.hpp"

#define check_range(str, start, end)    \
    do {                                \
        if ((start) >= (str).size())    \
            return false;               \
        if (start > end)                \
            return false;               \
        if ((end) > (str).size())       \
            (end) = (str).size();       \
    } while(0)

bool isVChar(char ch)
{
    return (ch >= '\x21' && ch <= '\x7E');
}

bool isObsText(char ch)
{
    unsigned char uch = static_cast<unsigned char>(ch);

    return (uch >= '\x80' && uch <= '\xFF');
}

bool isQdText(char ch)
{
    return (ch == '\t' || ch == ' ' || ch == '\x21' || (ch >= '\x23' && ch <= '\x5B') \
        || (ch >= '\x5D' && ch <= '\x7E') || isObsText(ch));
}

bool isWS(char ch)
{
    return (ch == '\t' || ch == ' ');
}

bool isDigit(char ch)
{
    return (ch >= '0' && ch <= '9');
}

bool isLowAlpha(char ch)
{
    return (ch >= 'a' && ch <= 'z');
}

bool isUpAlpha(char ch)
{
    return (ch >= 'A' && ch <= 'Z');
}

bool isAlpha(char ch)
{
    return (isLowAlpha(ch) || isUpAlpha(ch));
}

bool isAlphaNum(char ch)
{
    return (isAlpha(ch) || isDigit(ch));
}

bool isTChar(char ch)
{
    return (isDigit(ch) || isAlpha(ch) || ch == '!' || ch == '#' || ch == '$' || ch == '%' \
        || ch == '&' || ch == '\'' || ch == '*' || ch == '+' || ch == '-' || ch == '.' \
        || ch == '^' || ch == '_' || ch == '`' || ch == '|' || ch == '~');
}

bool isToken(const std::string& str, size_t start, size_t end)
{
    size_t pos = start;

    check_range(str, start, end);
    if (start == end)
        return false;
    while (pos != end)
        if (!isTChar(str[pos++]))
            return false;
    return true;
}

bool eatToken(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (p >= str.size() || p + 1 >= str.size())
        return false;
    while (p < str.size()) {
        if (!isTChar(str[p]))
            break;
        ++p;
    }
    if (p <= pos)
        return false;
    pos = p;
    return true;
}

void eatOWS(const std::string& str, size_t& pos)
{
    while (pos < str.size()) {
        if (!isWS(str[pos]))
            return;
        ++pos;
    }
}

bool isMark(char ch)
{
    return (ch == '-' || ch == '_' || '.' || '!' || '~' || '*' || '\'' || '(' || ')');
}

bool isHex(char ch)
{
    return (isDigit(ch) || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'));
}

bool isReserved(char ch)
{
    return (ch == ';' || ch == '/' || ch == '?' || ch == ':' || ch == '@' || ch == '&' \
        || ch == '=' || ch == '+' || ch == '$' || ch == ',');
}

bool isUnreserved(char ch)
{
    return (isAlphaNum(ch) || isMark(ch));
}

/**
 * 
 * 
 * 
 * 
*/

/*
bool isEscaped(const std::string& str, size_t pos)
{
    if (pos + 3 > str.size())
        return false;
    return (str[pos] == '%' && isHex(str[pos + 1]) && isHex(str[pos + 2]));
}

bool isUric(const std::string& str, size_t pos)
{
    return (isReserved(str[pos]) || isUnreserved(str[pos]) || isEscaped(str, pos));
}

bool isUricNoSlash(const std::string& str, size_t pos)
{
    return (isUnreserved(str[pos]) || isEscaped(str, pos) || str[pos] == ';' \
        || str[pos] == '?' || str[pos] == ':' || str[pos] == '@' || str[pos] == '&' \
        || str[pos] == '=' || str[pos] == '+' || str[pos] == '$' || str[pos] == ',');
}
*/

std::string& toLower(std::string& str)
{
    for (size_t i = 0; i < str.size(); ++i) {
        if (isUpAlpha(str[i]))
            str[i] += 0x20;
    }
    return str;
}

bool isCaseInsensitiveSame(const std::string& str1, const std::string& str2)
{
    std::string lowStr1 = str1, lowStr2 = str2;

    return (toLower(lowStr1) == toLower(lowStr2));
}

/**
 * 
 * 
 * 
 * 
*/

bool isOWS(const std::string& str, size_t start, size_t end)
{
    check_range(str, start, end);
    for (size_t i = start; i < end; ++i)
        if (!isWS(str[i]))
            return false;
    return true;
}

bool reatWS(const std::string& str, size_t& pos)
{
    for (; ; --pos) {
        if (!isWS(str[pos]))
            return true;
        if (pos == 0)
            return false;
    }
}

bool isType(const std::string& str, size_t start, size_t end)
{
    return isToken(str, start, end);
}

bool isSubType(const std::string& str, size_t start, size_t end)
{
    return isToken(str, start, end);
}

bool isQuotedPair(const std::string& str, size_t pos)
{
    if (pos + 2 >= str.size())
        return false;
    return (str[pos] == '\\' && (str[pos + 1] == '\t' || str[pos + 1] == ' ' \
        || isVChar(str[pos + 1]) || isObsText(str[pos + 1])));
}

bool eatQuotedString(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (!str[p++] == '\"')
        return false;
    while (p < str.size()) {
        if (isQdText(str[p])) {
            ++p;
            continue;
        }
        if (isQuotedPair(str, p)) {
            p += 2;
            continue;
        }
        break;
    }
    if (p >= str.size())
        return false;
    if (!str[p++] == '\"')
        return false;
    pos = p;
    return true;
}

bool eatParameter(const std::string& str, size_t& pos)
{
    size_t sep;

    if (!eatToken(str, pos))
        return false;
    if (pos + 2 >= str.size())
        return false;
    if (str[pos++] != '=')
        return false;
    if (str[pos] == '\"')
        return eatQuotedString(str, pos);
    else
        return eatToken(str, pos);
}

bool isMediaTypeRemain(const std::string& str, size_t pos)
{
    size_t sep;

    do {
        eatOWS(str, pos);
        if (pos == str.size())
            return false;
        if (str[pos] != ';')
            return false;
        ++pos;
        eatOWS(str, pos);
        if (pos == str.size())
            return false;
        if (!eatParameter(str, pos))
            return false;
    } while (pos < str.size());
    return true;
}

bool isMediaType(const std::string& str)
{
    size_t sep, subTypeEnd, remainPos;

    if ((sep = str.find('/')) == std::string::npos)
        return false;
    if ((subTypeEnd = str.find(';', sep + 1)) != std::string::npos) {
        --subTypeEnd;
        reatWS(str, subTypeEnd);
        ++subTypeEnd;
    }
    else
        subTypeEnd = str.size();

    return (isType(str, 0, sep) && isSubType(str, sep + 1, subTypeEnd) \
        && isMediaTypeRemain(str, subTypeEnd));
}
