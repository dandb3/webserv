#include "parse.hpp"

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

    if (start == end)
        return false;
    while (pos != end)
        if (!isTChar(str[pos++]))
            return false;
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


