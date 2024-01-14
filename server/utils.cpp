/**
 * 민관이형이 구현한 utils.cpp에 머지 할 예정.
 * 그냥 임시파일이다.
*/
#include <sstream>
#include "utils.hpp"

static std::string sizeToStr(size_t size)
{
    std::stringstream ss;
    std::string result;

    ss << size;
    ss >> result;
    return result;
}

static std::string ft_inet_ntoa(in_addr_t addr)
{
    std::stringstream ss;
    in_addr_t hAddr = ntohl(addr);
    u_char* addrP;

    addrP = reinterpret_cast<u_char*>(&hAddr);
    for (int i = 0; i < 4; ++i) {
        ss << addrP[i];
        if (i != 3)
            ss << '.';
    }
    return ss.str();
}