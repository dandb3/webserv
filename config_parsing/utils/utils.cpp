#include "utils.hpp"

// i는 실행 후 다음 단어의 시작 위치를 가리킴
std::string getWord(std::string const &file_content, size_t &i) {
    size_t pos = file_content.find_first_not_of(WHITESPACE, i);
    size_t nextPos = file_content.find_first_of(WHITESPACE, pos);
    if (pos == std::string::npos) { // 더 이상 단어가 없음
        i = std::string::npos;
        return "";
    }
    if (nextPos == std::string::npos) { // 마지막 단어
        i = nextPos;
        return file_content.substr(pos);
    }
    std::string ret = file_content.substr(pos, nextPos - pos);
    i = nextPos + 1;
    return ret;
}

// getKey에서 들어올 수 있는 key에 속하는지 확인 -> 아니면 에러
// key에 들어올 수 있는 값들의 목록
std::set<std::string> SERVER_KEY_SET = {
    "listen", "server_name", "root", "index", "autoindex", "client_max_body_size", \
    "error_page", "location", "access_log", "error_log", "cgi", "allow_methods", \
    "request_timeout", "keepalive_timeout"
};

std::string getKey(std::string const &file_content, size_t &i) {
    std::string key = getWord(file_content, i);
    if (key.find_first_of(SEPARATOR) != std::string::npos)
        throw std::runtime_error("config 파일 파싱 중 에러 발생");
    return key;
}

// delimiter를 만나기 전까지의 문자열을 split해서 반환
std::vector<std::string> getValue(std::string const &file_content, size_t &i, char delimiter) {
    std::vector<std::string> value;
    size_t end = file_content.find_first_of(delimiter, i);
    if (end == std::string::npos)
        throw std::runtime_error("config 파일 파싱 중 에러 발생: getValue실패");
    std::string str = file_content.substr(i, end - i);
    if (str.find_first_of("{};") != std::string::npos)
        throw std::runtime_error("config 파일 파싱 중 에러 발생: getValue실패");
    size_t pos = 0;
    std::string word;

    while (pos != std::string::npos && pos < str.size()) {
        word = getWord(str, pos);
        if (word == "")
            break;
        value.push_back(word);
    }
    i = end + 1;
    return value;
}

int ft_inet_aton(const char *str, struct in_addr *addr) {
    int i, len, n;
    uint32_t ip;
    char c;

    if (!strcmp(str, "0.0.0.0")) {
        addr->s_addr = INADDR_ANY;
        return 1;
    }
    ip = 0;
    for (i = 0; i < 4; i++) {
        n = 0;
        ip >>= 8;
        for (len = 0; len < 4; len++) {
            if (str[len] == '.' || str[len] == '\0')
                break;
        }
        if (len == 0 || len == 4)
            return 0;
        if ((i == 3 && str[len] == '.') || (i != 3 && str[len] == '\0'))
            return 0;

        c = *str++;
        // n에 숫자 담기
        while (len--) {
            if (c < '0' || c > '9')
                return 0;
            n = (n * 10) + c - '0';
            c = *str++;
        }
        if (i == 0 && n == 0)
            return 0;
        if (n > 255)
            return 0;
        ip = ip | (n << 24);
    }
    addr->s_addr = ip;
    return 1;
}

void printParsedServer(ServerConfig &server) {
    std::cout << "======================" << std::endl;
    std::cout << "[parseServer] " << "portsWithINADDR_ANY: ";
    for (std::vector<int>::iterator it = ServerConfig::portsWithINADDR_ANY.begin(); it != ServerConfig::portsWithINADDR_ANY.end(); it++)
        std::cout << *it << " ";
    std::cout << std::endl;
    std::cout << "[parseServer] " << "ip: " << server.getIp().s_addr << std::endl;
    std::cout << "[parseServer] " << "port: " << server.getPort() << std::endl;
    std::cout << "======================" << std::endl;
}
