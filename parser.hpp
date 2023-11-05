#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>

# define HTTP_NAME "HTTP"
# define CRLF "\r\n"
# define SP ' '
# define HTAB '\t'

# define DIGIT "0123456789"
# define HEXDIG DIGIT "ABCDEF"
# define ALPHA "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
# define TCHAR "!#$%&'*+-.^_`|~" DIGIT ALPHA
# define UNRESERVED ALPHA DIGIT "-._~"
# define SUB_DELIMS "!$&'()*+,;="

# define MAX_REQUEST_LEN 8000

/* ---------------- inline functions ---------------- */

static inline
bool is_size_fit(const std::string& str, size_t pos, size_t size)
{
    return (str.size() - pos >= size);
}

static inline
bool is_same(char c1, char c2)
{
    return (c1 == c2);
}

static inline
bool is_between(char c, char low, char high)
{
	return (c >= low && c <= high);
}

static inline
bool is_WS(char c)
{
	return (is_same(c, SP) || is_same(c, HTAB));
}

static inline
bool is_tchar(char c)
{
	return (strchr(TCHAR, c) != NULL);
}

static inline
bool is_VCHAR(char c)
{
	return (is_between(c, 0x21, 0x7E));
}

static inline
bool is_unreserved(char c)
{
	return (strchr(UNRESERVED, c) != NULL);
}

static inline
bool is_HEXDIG(char c)
{
	return (strchr(HEXDIG, c) != NULL);
}

static inline
bool is_sub_delims(char c)
{
	return (strchr(SUB_DELIMS, c) != NULL);
}

static inline
bool is_pct_encoded(const std::string& uri, size_t pos)
{
	if (!is_size_fit(uri, pos, 3))
        return false;
    if (is_same(uri[pos], '%') && is_HEXDIG(uri[pos + 1]) \
        && is_HEXDIG(uri[pos + 2]))
        return true;
    return false;
}

static inline
bool is_pchar(const std::string& str, size_t& pos)
{
    if (is_unreserved(str[pos]) || is_sub_delims(str[pos] \
        || is_same(str[pos], ':') || is_same(str[pos], '@'))) {
        ++pos;
        return true;
    }
    if (is_pct_encoded(str, pos)) {
        pos += 3;
        return true;
    }
    return false;
}

static inline
bool is_segment(const std::string& str)
{
    size_t pos = 0;

    while (pos < str.size())
        if (!is_pchar(str, pos))
            return false;
    return true;
}

static inline
bool is_dec_octet(const std::string& str)
{
	if (str.size() == 1 && isdigit(str[0]))
		return true;
	if (str.size() == 2 && is_between(str[0], '1', '9') && isdigit(str[1]))
		return true;
	if (str.size() == 3) {
		if (is_same(str[0], '1') && isdigit(str[1]) && isdigit(str[2]))
			return true;
		if (is_same(str[0], '2') && is_between(str[1], '0', '4') && isdigit(str[2]))
			return true;
		if (is_same(str[0], '2') && is_same(str[1], '5') && is_between(str[2], '0', '5'))
			return true;
	}
	return false;
}

static inline
bool is_h16(const std::string& str)
{
	if (str.size() < 1 || str.size() > 4)
		return false;
	for (size_t i = 0; i < str.size(); ++i)
		if (!is_HEXDIG(str[i]))
			return false;
	return true;
}

#endif
