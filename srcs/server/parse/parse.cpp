#include "parse.hpp"

#define checkNcharAvailable(str, pos, n)       \
    do {                                        \
        if ((pos) >= (str).size())              \
            return false;                       \
        if ((pos) + (n) > (str).size())         \
            return false;                       \
    } while (0)

static bool isDigit(char ch)
{
    return (ch >= '0' && ch <= '9');
}

static bool isLowAlpha(char ch)
{
    return (ch >= 'a' && ch <= 'z');
}

static bool isUpAlpha(char ch)
{
    return (ch >= 'A' && ch <= 'Z');
}

static bool isAlpha(char ch)
{
    return (isLowAlpha(ch) || isUpAlpha(ch));
}

static bool isAlphaNum(char ch)
{
    return (isAlpha(ch) || isDigit(ch));
}

static bool isChar(char ch)
{
    return (ch >= '\x00' && ch <= '\x7F');
}

static bool isCTL(char ch)
{
    return ((ch >= '\x00' && ch <= '\x1F') || ch == '\x7F');
}

static std::string& toLower(std::string& str)
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

static bool isLWSP(char ch)
{
    return (ch == ' ' || ch == '\t' || ch == '\n');
}

static bool isText(char ch)
{
    if (!isChar(ch))
        return false;
    if (isLWSP(ch) || ch == '\r')
        return true;
    if (isCTL(ch))
        return false;
    return true;
}

static bool isQdText(char ch)
{
    if (!isChar(ch))
        return false;
    if (isLWSP(ch))
        return true;
    if (isCTL(ch) || ch == '\"')
        return false;
    return true;
}

static bool isWS(char ch)
{
    return (ch == '\t' || ch == ' ');
}

static bool isSeparator(char ch)
{
    return (ch == '(' || ch == ')' || ch == '<' || ch == '>' || ch == '@' || ch == ',' \
        || ch == ';' || ch == ':' || ch == '\\' || ch == '\"' || ch == '/' || ch == '[' \
        || ch == ']' || ch == '?' || ch == '=' || ch == '{' || ch == '}' || ch == ' ' \
        || ch == '\t');
}

static bool eatToken(const std::string& str, size_t& pos)
{
    size_t p = pos;

    while (p < str.size()) {
        if (!isChar(str[p]) || isCTL(str[p]) || isSeparator(str[p]))
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

static bool isMark(char ch)
{
    return (ch == '-' || ch == '_' || ch == '.' || ch == '!' || ch == '~' || ch == '*' \
        || ch == '\'' || ch == '(' || ch == ')');
}

static bool isHex(char ch)
{
    return (isDigit(ch) || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'));
}

static bool isReserved(char ch)
{
    return (ch == ';' || ch == '/' || ch == '?' || ch == ':' || ch == '@' || ch == '&' \
        || ch == '=' || ch == '+' || ch == '$' || ch == ',' || ch == '[' || ch == ']');
}

static bool isUnreserved(char ch)
{
    return (isAlpha(ch) || isDigit(ch) || isMark(ch));
}

static bool isExtra(char ch)
{
    return (ch == ':' || ch == '@' || ch == '&' || ch == '=' || ch == '+' || ch == '$' \
        || ch == ',');
}

static bool isEscaped(const std::string& str, size_t pos)
{
    checkNcharAvailable(str, pos, 3);
    return (str[pos] == '%' && isHex(str[pos + 1]) && isHex(str[pos + 2]));
}

static bool eatUnitPchar(const std::string& str, size_t& pos)
{
    checkNcharAvailable(str, pos, 1);
    if (isUnreserved(str[pos]) || isExtra(str[pos])) {
        ++pos;
        return true;
    }
    if (isEscaped(str, pos)) {
        pos += 3;
        return true;
    }
    return false;
}

static bool eatUnitUric(const std::string& str, size_t& pos)
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

static bool eatUnitUricNoSlash(const std::string& str, size_t& pos)
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

static bool eatType(const std::string& str, size_t& pos)
{
    return eatToken(str, pos);
}

static bool eatSubType(const std::string& str, size_t& pos)
{
    return eatToken(str, pos);
}

/**
 * RFC 3875에 의한 quoted-string.
 * RFC 7230에서는 좀 다르다. qdtext | quoted-pair 로 구분됨.
*/
static bool eatQuotedString(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (p >= str.size())
        return false;
    if (str[p++] != '\"')
        return false;
    while (p < str.size()) {
        if (!isQdText(str[p]))
            break;
        ++p;
    }
    if (p >= str.size())
        return false;
    if (str[p++] != '\"')
        return false;
    pos = p;
    return true;
}

static bool eatParameter(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (!eatToken(str, p))
        return false;
    if (p >= str.size())
        return false;
    if (str[p++] != '=')
        return false;
    if (!eatQuotedString(str, p) && !eatToken(str, p))
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

static void eatSegment(const std::string& str, size_t& pos)
{
    while (pos < str.size())
        if (!eatUnitPchar(str, pos))
            break;
}

static void eatPathSegments(const std::string& str, size_t& pos)
{
    eatSegment(str, pos);
    while (pos < str.size()) {
        if (str[pos] != '/')
            break;
        ++pos;
        eatSegment(str, pos);
    }
}

static bool eatAbsPath(const std::string& str, size_t& pos)
{
    size_t p = pos;

    checkNcharAvailable(str, pos, 1);
    if (str[p++] != '/')
        return false;
    eatPathSegments(str, p);
    pos = p;
    return true;
}

static void eatQueryString(const std::string& str, size_t& pos)
{
    while (pos < str.size())
        if (!eatUnitUric(str, pos))
            break;
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

static bool eatScheme(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (p >= str.size())
        return false;
    if (!isAlpha(str[p++]))
        return false;
    while (p < str.size()) {
        if (!isAlpha(str[p]) && !isDigit(str[p]) && str[p] != '+' && str[p] != '-' \
            && str[p] != '.')
            break;
        ++p;
    }
    pos = p;
    return true;
}

static void eatUserInfo(const std::string& str, size_t& pos)
{
    while (pos < str.size()) {
        if (isUnreserved(str[pos]) || str[pos] == ';' || str[pos] == ':' || str[pos] == '&' \
            || str[pos] == '=' || str[pos] == '+' || str[pos] == '$' || str[pos] == ',') {
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

static bool eatDomainlabel(const std::string& str, size_t& pos)
{
    size_t p, result;

    checkNcharAvailable(str, pos, 1);
    if (!isAlphaNum(str[pos]))
        return false;
    result = p = ++pos;
    while (p < str.size()) {
        if (isAlphaNum(str[p]))
            result = p + 1;
        else if (str[p] != '-')
            break;
        ++p;
    }
    if (result <= pos)
        return false;
    pos = result;
    return true;
}

static bool eatToplabel(const std::string& str, size_t& pos)
{
    size_t p, result;

    checkNcharAvailable(str, pos, 1);
    if (!isAlpha(str[pos]))
        return false;
    result = p = ++pos;
    while (p < str.size()) {
        if (isAlphaNum(str[p]))
            result = p + 1;
        else if (str[p] != '-')
            break;
        ++p;
    }
    if (result <= pos)
        return false;
    pos = result;
    return true;
}

static bool eatHostname(const std::string& str, size_t& pos)
{
    size_t p = pos, result = pos;

    while (p < str.size()) {
        if (eatToplabel(str, p))
            result = p;
        else if (!eatDomainlabel(str, p))
            break;
        if (p >= str.size() || str[p] != '.')
            break;
        ++p;
    }
    if (result <= pos)
        return false;
    if (result < str.size() && str[result] == '.')
        ++result;
    pos = result;
    return true;
}

static bool eatIPv4address(const std::string& str, size_t& pos)
{
    size_t p = pos, tmp = pos;

    for (int i = 0; i < 4; ++i) {
        while (p < str.size()) {
            if (!isDigit(str[p]))
                break;
            ++p;
        }
        if (p == tmp)
            return false;
        if (i == 3)
            break;
        if (p >= str.size() || str[p] != '.')
            return false;
        tmp = ++p;
    }
    pos = p;
    return true;
}

static bool eatHost(const std::string& str, size_t& pos)
{
    return (eatHostname(str, pos) || eatIPv4address(str, pos));
}

static void eatPort(const std::string& str, size_t& pos)
{
    while (pos < str.size()) {
        if (!isDigit(str[pos]))
            break;
        ++pos;
    }
}

static bool eatHostport(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (!eatHost(str, p))
        return false;
    if (p >= str.size() || str[p] != ':') {
        pos = p;
        return true;
    }
    ++p;
    eatPort(str, p);
    pos = p;
    return true;
}

static void eatServer(const std::string& str, size_t& pos)
{
    size_t p = pos;

    eatUserInfo(str, p);
    if (p < str.size() && str[p] == '@') {
        ++p;
        pos = p;
    }
    eatHostport(str, pos);
}

static bool eatRegName(const std::string& str, size_t& pos)
{
    size_t p = pos;

    while (p < str.size()) {
        if (isUnreserved(str[p]) || str[p] == '$' || str[p] == ',' || str[p] == ';' \
            || str[p] == ':' || str[p] == '@' || str[p] == '&' || str[p] == '=' \
            || str[p] == '+') {
            ++p;
            continue;
        }
        if (isEscaped(str, p)) {
            p += 3;
            continue;
        }
        break;
    }
    if (p <= pos)
        return false;
    pos = p;
    return true;
}

static void eatAuthority(const std::string& str, size_t& pos)
{
    size_t p = pos;

    eatServer(str, p);
    if (p <= pos)
        eatRegName(str, p);
    pos = p;
}

static bool eatNetPath(const std::string& str, size_t& pos)
{
    size_t p = pos;

    checkNcharAvailable(str, p, 2);
    if (str[p] != '/' || str[p + 1] != '/')
        return false;
    p += 2;
    eatAuthority(str, p);
    eatAbsPath(str, p);
    pos = p;
    return true;
}

static void eatQuery(const std::string& str, size_t& pos)
{
    while (pos < str.size())
        if (!eatUnitUric(str, pos))
            break;
}

static bool eatHierPart(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (!eatNetPath(str, p) && !eatAbsPath(str, p))
        return false;
    if (p >= str.size() || str[p] != '?') {
        pos = p;
        return true;
    }
    ++p;
    eatQuery(str, p);
    pos = p;
    return true;
}

static bool eatOpaquePart(const std::string& str, size_t& pos)
{
    size_t p = pos;

    if (!eatUnitUricNoSlash(str, p))
        return false;
    while (p < str.size())
        if (!eatUnitUric(str, p))
            break;
    pos = p;
    return true;
}

static bool eatAbsoluteURI(const std::string& str, size_t& pos)
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
    pos = p;
    return true;
}

static void eatFragment(const std::string& str, size_t& pos)
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
    checkNcharAvailable(str, pos, 3);
    return (isDigit(str[pos]) && isDigit(str[pos + 1]) && isDigit(str[pos + 2]));
}

void eatReasonPhrase(const std::string& str, size_t& pos)
{
    while (pos < str.size())
        if (!isText(str[pos++]))
            break;
}

static bool eatFieldName(const std::string& str, size_t& pos)
{
    return eatToken(str, pos);
}

static void eatFieldContent(const std::string& str, size_t& pos)
{
    while (pos < str.size()) {
        if (isSeparator(str[pos])) {
            ++pos;
            continue;
        }
        if (!eatToken(str, pos) && !eatQuotedString(str, pos))
            break;
    }
}

static void eatFieldValue(const std::string& str, size_t& pos)
{
    while (pos < str.size()) {
        eatFieldContent(str, pos);
        if (pos >= str.size() || !isLWSP(str[pos]))
            break;
        ++pos;
    }
}

bool isGenericField(const std::string& str)
{
    size_t pos = 0;

    if (!eatFieldName(str, pos))
        return false;
    if (pos >= str.size() || str[pos] != ':')
        return false;
    ++pos;
    eatFieldValue(str, pos);
    if (pos < str.size())
        return false;
    return true;
}
