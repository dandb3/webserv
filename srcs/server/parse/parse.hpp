#ifndef PARSE_HPP
#define PARSE_HPP

#include <string>

#define CRLF "\r\n"
#define WS "\t "

typedef std::pair<std::string, std::string> pair_t;

bool isCaseInsensitiveSame(const std::string& str1, const std::string& str2);

/**
 * 규칙
 * 1. bool eat*(const std::string& str, size_t& pos)
 *     말 그대로 str의 pos에서 시작해서 *에 해당하는 char 만큼을 이동한다.
 *     성공적으로 *을 읽어들였다면 true 리턴, pos 값도 *을 읽어들인 이후로 이동함.
 *     나머지는 false 리턴, pos는 변하지 않는다.
 * 
 * 2. void eat*(const std::string& str, size_t& pos)
 *     이 경우는 *이 empty일 가능성이 있는 경우이다.
 *     즉, '성공적으로 읽어들였다' 라는 것이 의미가 없는 경우에 해당.
 *     그래서 1번 함수의 true일 경우와 동일하게 동작한다.
 * 
 * 3. bool is*()
 *     *에 해당하는지 아닌지 판별.
 *     그냥 true / false만 리턴해 준다.
*/

void eatOWS(const std::string& str, size_t& pos);
bool isGenericField(const std::string& str);
bool isMediaType(const std::string& str);
bool isLocalPathquery(const std::string& str);
bool isFragmentURI(const std::string& str);
bool isStatusCode(const std::string& str, size_t pos);
void eatReasonPhrase(const std::string& str, size_t& pos);

#endif
