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

inline
bool isWS(char c)
{
	return (c == SP || c == HTAB);
}

inline
bool isTCHAR(char c)
{
	return (strchr(TCHAR, c) != NULL);
}

inline
bool isVCHAR(char c)
{
	return (c >= 0x21 && c <= 0x7E);
}

inline
bool isUNRESERVED(char c)
{
	return (strchr(UNRESERVED, c) != NULL);
}

inline
bool isHEXDIG(char c)
{
	return (strchr(HEXDIG, c) != NULL);
}

inline
bool isSUBDELIMS(char c)
{
	return (strchr(SUB_DELIMS, c) != NULL);
}

#endif
