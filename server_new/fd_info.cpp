#include "fd_info.hpp"

info::info(int sockfd)
: http_sock(sockfd)
{}

fd_info::fd_info(int type, struct info* info)
: _type(static_cast<char>(type)), _info(info)
{}
