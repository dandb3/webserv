#include "parse.hpp"

bool isText(char ch)
{
    return ((ch >= '\x20' && ch <= '\x7E') || ch == '\t' || ch == '\n' || ch == '\r');
}

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

bool isExtra(char ch)
{
    return (ch == ':' || ch == '@' || ch == '&' || ch == '=' || ch == '+' || ch == '$' \
        || ch == ',');
}

bool isEscaped(const std::string& str, size_t pos)
{
    if (pos + 3 > str.size())
        return false;
    return (str[pos] == '%' && isHex(str[pos + 1]) && isHex(str[pos + 2]));
}

bool eatUnitUric(const std::string& str, size_t& pos)
{
    if (isReserved(str[pos]) || isUnreserved(str[pos])) {
        ++pos;
        return true;
    }
    if (isEscaped(str, pos)) {
        pos += 3;
        return true;
    }
    return false;
}

bool eatUnitUricNoSlash(const std::string& str, size_t& pos)
{
    if (isEscaped(str, pos)) {
        pos += 3;
        return true;
    }
    if (isUnreserved(str[pos]) || str[pos] == ';' || str[pos] == '?' \
        || str[pos] == ':' || str[pos] == '@' || str[pos] == '&' \
        || str[pos] == '=' || str[pos] == '+' || str[pos] == '$' \
        || str[pos] == ',') {
        ++pos;
        return true;
    }
    return false;
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

bool eatType(const std::string& str, size_t& pos)
{
    return eatToken(str, pos);
}

bool eatSubType(const std::string& str, size_t& pos)
{
    return eatToken(str, pos);
}

bool isQuotedPair(const std::string& str, size_t pos)
{
    if (pos + 2 >= str.size())
        return false;
    return (str[pos] == '\\' && (str[pos + 1] == '\t' || str[pos + 1] == ' ' \
        || isVChar(str[pos + 1]) || isObsText(str[pos + 1])));
}

/**
 * RFC 3875에 의한 quoted-string.
 * RFC 7230에서는 좀 다르다. qdtext | quoted-pair 로 구분됨.
*/
bool eatQuotedString(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (p >= str.size())
        return false;
    if (str[p++] != '\"')
        return false;
    while (p < str.size()) {
        if (!isQdText(str[p++]))
            break;
    }
    if (p >= str.size())
        return false;
    if (str[p++] != '\"')
        return false;
    pos = p;
    return true;
}

bool eatParameter(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (!eatToken(str, p))
        return false;
    if (p >= str.size())
        return false;
    if (str[p++] != '=')
        return false;
    if (!eatQuotedString(str, p) || !eatToken(str, p))
        return false;
    pos = p;
    return true;
}

bool isMediaType(const std::string& str)
{
    size_t pos = 0;

    if (!eatType(str, pos))
        return false;
    if (pos >= str.size())
        return false;
    if (str[pos++] != '/')
        return false;
    if (!eatSubType(str, pos))
        return false;
    while (pos < str.size()) {
        eatOWS(str, pos);
        if (pos >= str.size())
            return false;
        if (str[pos++] != ';')
            return false;
        eatOWS(str, pos);
        if (!eatParameter(str, pos))
            return false;
    }
    return true;
}

void eatSegment(const std::string& str, size_t& pos)
{
    while (pos < str.size()) {
        if (isUnreserved(str[pos]) || isExtra(str[pos])) {
            ++pos;
            continue;
        }
        if (isEscaped(str, pos)) {
            pos += 3;
            continue;
        }
        break;
    }
}

bool eatPathSegments(const std::string& str, size_t& pos)
{
    size_t p = pos;

    eatSegment(str, p);
    while (p < str.size()) {
        if (str[p++] != '/') {
            pos = p;
            return true;
        }
        eatSegment(str, p);
    }
    pos = p;
    return true;
}

bool eatAbsPath(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (p >= str.size())
        return false;
    if (str[p++] != '/')
        return false;
    if (!eatPathSegments(str, p))
        return false;
    pos = p;
    return true;
}

void eatQueryString(const std::string& str, size_t& pos)
{
    while (pos < str.size()) {
        if (!eatUnitUric(str, pos))
            break;
    }
}

bool isLocalPathquery(const std::string& str)
{
    size_t pos = 0;

    if (!eatAbsPath(str, pos))
        return false;
    if (pos >= str.size())
        return true;
    if (str[pos++] != '?')
        return false;
    eatQueryString(str, pos);
    if (pos < str.size())
        return false;
    return true;
}

bool eatScheme(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (p >= str.size())
        return false;
    if (!isAlpha(str[p++]))
        return false;
    while (p < str.size())
        if (!isAlpha(str[p]) && !isDigit(str[p]) && str[p] != '+' && str[p] != '-' \
            || str[p] != '.')
            break;
    if (p < str.size())
        return false;
    pos = p;
    return true;
}

bool eatNetPath(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (p >= str.size() || p + 2 > str.size())
        return false;
    if (str[p] != '/' || str[p + 1] != '/')
        return false;
    p += 2;
    if (!eatAuthority(str, p))
        return false;
    eatAbsPath(str, p);
    pos = p;
    return true;
}

void eatQuery(const std::string& str, size_t& pos)
{
    eatQueryString(str, pos);
}

bool eatHierPart(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (!eatNetPath(str, p) && !eatAbsPath(str, p))
        return false;
    if (p >= str.size() || str[p] != '?') {
        pos = p;
        return true;
    }
    eatQuery(str, p);
    if (p < str.size())
        return false;
    pos = p;
    return true;
}

bool eatAbsoluteURI(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (!eatScheme(str, p))
        return false;
    if (p >= str.size())
        return false;
    if (str[p++] != ':')
        return false;
    if (!eatHierPart(str, p) && !eatOpaquePart(str, p))
        return false;
    if (p < str.size())
        return false;
    pos = p;
    return true;
}

void eatFragment(const std::string& str, size_t& pos)
{
    while (pos < str.size())
        if (!eatUnitUric(str, pos))
            break;
}

bool isFragmentURI(const std::string& str)
{
    size_t pos = 0;

    if (!eatAbsoluteURI(str, pos))
        return false;
    if (pos == str.size())
        return true;
    if (str[pos++] != '#')
        return false;
    eatFragment(str, pos);
    if (pos < str.size())
        return false;
    return true;
}

bool isStatusCode(const std::string& str, size_t pos)
{
    if (pos >= str.size() || pos + 3 > str.size())
        return false;
    return (isDigit(str[pos]) && isDigit(str[pos + 1]) && isDigit(str[pos + 2]));
}

void eatReasonPhrase(const std::string& str, size_t& pos)
{
    while (pos < str.size())
        if (!isText(str[pos++]))
            break;
}

bool isStatus(const pair_t& p)
{
    const std::string& name = p.first, value = p.second;
    size_t pos = 0;

    if (!isCaseInsensitiveSame(name, "Status"))
        return false;
    if (!isStatusCode(value, pos))
        return false;
    pos += 3;
    if (pos >= value.size())
        return false;
    if (value[pos++] != ' ')
        return false;
    eatReasonPhrase(value, pos);
    if (pos < value.size())
        return false;
    return true;
}
