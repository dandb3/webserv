#include "parse.hpp"

bool isTChar(char ch)
{
    return (isDigit(ch) || isAlpha(ch) || ch == '!' || ch == '#' || ch == '$' || ch == '%' \
        || ch == '&' || ch == '\'' || ch == '*' || ch == '+' || ch == '-' || ch == '.' \
        || ch == '^' || ch == '_' || ch == '`' || ch == '|' || ch == '~');
}
