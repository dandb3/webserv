#include <cstring>
#include "parser.hpp"

/* ---------------- static functions ---------------- */

static
bool is_absolute_path(const std::string& str)
{
    size_t seg_start = 1, seg_end;

    if (str.empty() || !is_same(str[0], '/'))
        return false;
    do {
        seg_end = str.find('/', seg_start);
        if (!is_segment(str.substr(seg_start, seg_end)))
            return false;
        seg_start = seg_end + 1;
    } while (seg_end != std::string::npos);
    return true;
}

static
bool is_query(const std::string& str)
{
    size_t pos = 0;

    while (pos < str.size()) {
        if (is_same(str[pos], '/') || is_same(str[pos], '?'))
            ++pos;
        else if (!is_pchar(str, pos))
            return false;
    }
    return true;
}

static
bool is_userinfo(const std::string& str)
{
	size_t pos = 0;

	while (pos < str.size()) {
		if (is_unreserved(str[pos]) || is_sub_delims(str[pos]) \
			|| is_same(str[pos], ':'))
			++pos;
		else if (is_pct_encoded(str, pos))
			pos += 3;
		else
			return false;
	}
	return true;
}

static
bool is_port(const std::string& str)
{
    size_t pos = 0;

    while (pos < str.size()) {
        if (!isdigit(str[pos]))
            return false;
        ++pos;
    }
    return true;
}

static
bool is_reg_name(const std::string& str)
{
    size_t pos = 0;

    while (pos < str.size()) {
        if (is_unreserved(str[pos]) || is_sub_delims(str[pos]))
            ++pos;
        else if (is_pct_encoded(str, pos))
            pos += 3;
        else
            return false;
    }
    return true;
}

static
bool is_IPv4address(const std::string& str)
{
    size_t sep1, sep2, sep3;

    if ((sep1 = str.find('.')) == std::string::npos)
        return false;
    if ((sep2 = str.find('.', sep1 + 1)) == std::string::npos)
        return false;
    if ((sep3 = str.find('.', sep2 + 1)) == std::string::npos)
        return false;
    return (is_dec_octet(str.substr(0, sep1)) && is_dec_octet(str.substr(sep1 + 1, sep2)) \
        && is_dec_octet(str.substr(sep2 + 1, sep3)) && is_dec_octet(str.substr(sep3 + 1)));
}

static
bool is_ls32(const std::string& str)
{
    size_t sep;

	if (is_IPv4address(str))
        return true;
    if ((sep = str.find(':')) == std::string::npos)
        return false;
    if (!is_h16(str.substr(0, sep)) || !is_h16(str.substr(sep + 1)))
        return false;
    return true;
}

static
int h16_cnt(const std::string& str)
{
    int cnt = 0;
    size_t sep1 = 0, sep2 = 0;

    while (sep2 != std::string::npos) {
        sep2 = str.find(':', sep1);
        if (!is_h16(str.substr(sep1, sep2)))
            return -1;
        ++cnt;
        sep1 = sep2 + 1;
    }
    return cnt;
}

static
int h16_ls32_cnt(const std::string& str)
{
    int cnt;
    size_t sep;

    sep = str.rfind(':');
    if (sep == std::string::npos) {
        if (is_ls32(str))
            return 2;
        return h16_cnt(str);
    }
    if (is_ls32(str.substr(sep + 1))) {
        if ((cnt = h16_cnt(str.substr(0, sep))) == -1)
            return -1;
        cnt += 2;
    }
    else
        cnt = h16_cnt(str);
    return cnt;
}

static
bool is_IPv6address(const std::string& str)
{
    size_t sep;
    int front_cnt, back_cnt;

    if ((sep = str.find("::")) == std::string::npos) {
        if (h16_cnt(str) == 8)
            return true;
        return false;
    }
    if ((back_cnt = h16_ls32_cnt(str.substr(sep + 2))) == -1 \
        || (front_cnt = h16_cnt(str.substr(0, sep))) == -1)
        return false;
    if (front_cnt + back_cnt <= 7)
        return true;
    return false;
}

static
bool is_IPvFuture(const std::string& str)
{
    size_t sep, i = 1;

    if (str.empty() || !is_same(str[0], 'v'))
        return false;
    if ((sep = str.find('.')) == std::string::npos)
        return false;
    do {
        if (!is_HEXDIG(str[i]))
            return false;
    } while (i < sep);
    if (++i >= str.size())
        return false;
    do {
        if (!is_unreserved(str[i]) || !is_sub_delims(str[i]) \
            || !is_same(str[i], ':'))
            return false;
    } while (i < str.size());
    return true;
}

static
bool is_IP_literal(const std::string& str)
{
    if (str.empty() || !is_same(str[0], '[') \
        || !is_same(str[str.size() - 1], ']'))
        return false;
    std::string ip = str.substr(1, str.size() - 1);
    return (is_IPv6address(ip) || is_IPvFuture(ip));
}

static
bool is_host(const std::string& str)
{
    return (is_IP_literal(str) || is_IPv4address(str) \
        || is_reg_name(str));
}

static
bool is_authority(const std::string& str)
{
    size_t sep1, sep2;

    if ((sep1 = str.find('@')) == std::string::npos)
        sep1 = 0;
    else if (!is_userinfo(str.substr(0, sep1)))
        return false;
    if ((sep2 = str.rfind(':')) != std::string::npos \
        && !is_port(str.substr(sep2 + 1)))
        sep2 = std::string::npos;
    return is_host(str.substr(sep1, sep2));
}

static
bool is_path_abempty(const std::string& str)
{
    size_t seg_start = 1, seg_end;

    if (str.empty())
        return true;
    if (!is_same(str[0], '/'))
        return false;
    do {
        seg_end = str.find('/', seg_start);
        if (!is_segment(str.substr(seg_start, seg_end)))
            return false;
        seg_start = seg_end + 1;
    } while (seg_end != std::string::npos);
    return true;
}

static
bool is_scheme(const std::string& str)
{
    size_t pos = 1;

    if (str.empty() || !isalpha(str[0]))
        return false;
    while (pos < str.size()) {
        if (!isalnum(str[pos]) && !is_same(str[pos], '+') \
            && !is_same(str[pos], '-') && !is_same(str[pos], '.'))
            return false;
        ++pos;
    }
    return true;
}

static
bool is_hier_part(const std::string& str)
{
    size_t sep;

    if (!is_size_fit(str, 0, 2) || !is_same(str[0], '/') \
        || !is_same(str[1], '/'))
        return false;
    if ((sep = str.find('/', 2)) == std::string::npos)
        return is_authority(str.substr(2));
    return (is_authority(str.substr(2, sep)) && is_path_abempty(str.substr(sep + 1)));
}

static
bool is_absolute_URI(const std::string& str)
{
    size_t sep1, sep2;

    if ((sep1 = str.find(':')) == std::string::npos)
        return false;
    if ((sep2 = str.find('?', sep1 + 1)) == std::string::npos)
        return (is_scheme(str.substr(0, sep1)) && is_hier_part(str.substr(sep1 + 1, sep2)));
    return (is_scheme(str.substr(0, sep1)) && is_hier_part(str.substr(sep1 + 1, sep2)) \
        && is_query(str.substr(sep2 + 1)));
}

/* ---------------- non-static functions ---------------- */

bool is_origin_form(const std::string& str)
{
    size_t sep = str.find('?');

    if (sep == std::string::npos)
        return (is_absolute_path(str));
    return (is_absolute_path(str.substr(0, sep)) && is_query(str.substr(sep + 1)));
}

bool is_absolute_form(const std::string& str)
{
    return is_absolute_URI(str);
}
