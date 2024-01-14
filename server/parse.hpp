#ifndef PARSE_HPP
#define PARSE_HPP

#include <string>

#define CRLF "\r\n"
#define WS "\t "

bool isCaseInsensitiveSame(const std::string& str1, const std::string& str2);

/**
 * 규칙
 * 1. const std::string& str, size_t start, size_t end 가 인자인 경우
 *     end는 std::string::npos인 경우 str.size()로 치환된다.
 *     start가 str.size()보다 크거나 같은지 체크
 * 2. const std::string& str, size_t& pos 가 인자인 경우
 *     start가 str.size()보다 크거나 같은지 체크
 *     true가 리턴될 경우에만 pos가 변화함.
 * 3. char ch 가 인자인 경우
 *     그냥 걔만 확인.
*/

void eatOWS(const std::string& str, size_t& pos);
bool reatWS(const std::string& str, size_t& pos);
bool isMediaType(const std::string& str);

#endif