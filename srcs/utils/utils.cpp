#include <sstream>
#include "utils.hpp"

int stringToInt(const std::string& str)
{
    std::stringstream ss;
    int result;

    ss << str;
    ss >> result;
    return result;
}

std::string sizeToStr(size_t size)
{
    std::stringstream ss;
    std::string result;

    ss << size;
    ss >> result;
    return result;
}

std::string ft_itoa(int num)
{
    std::stringstream ss;
    std::string result;

    ss << num;
    ss >> result;
    return result;
}

std::string ft_inet_ntoa(in_addr_t addr)
{
    std::stringstream ss;
    in_addr_t hAddr = ntohl(addr);
    u_char *addrP;

    addrP = reinterpret_cast<u_char *>(&hAddr);
    for (int i = 0; i < 4; ++i) {
        ss << addrP[i];
        if (i != 3)
            ss << '.';
    }
    return ss.str();
}

// i는 실행 후 다음 단어의 시작 위치를 가리킴
std::string getWord(std::string const &fileContent, size_t &i) {
    size_t pos = fileContent.find_first_not_of(WHITESPACE, i);
    size_t nextPos = fileContent.find_first_of(WHITESPACE, pos);
    if (pos == std::string::npos) { // 더 이상 단어가 없음
        i = std::string::npos;
        return "";
    }
    if (nextPos == std::string::npos) { // 마지막 단어
        i = nextPos;
        return fileContent.substr(pos);
    }
    std::string ret = fileContent.substr(pos, nextPos - pos);
    i = nextPos + 1;
    return ret;
}

std::string getKey(std::string const &fileContent, size_t &i) {
    std::string key = getWord(fileContent, i);
    if (key.find_first_of(SEPARATOR) != std::string::npos)
        throw std::runtime_error("config 파일 파싱 중 에러 발생");
    return key;
}

// delimiter를 만나기 전까지의 문자열을 split해서 반환
std::vector<std::string> getValues(std::string const &fileContent, size_t &i, char delimiter) {
    std::vector<std::string> values;
    size_t end = fileContent.find_first_of(delimiter, i);
    if (end == std::string::npos)
        throw std::runtime_error("config 파일 파싱 중 에러 발생: getValues실패");
    std::string str = fileContent.substr(i, end - i);
    if (str.find_first_of("{};") != std::string::npos)
        throw std::runtime_error("config 파일 파싱 중 에러 발생: getValues실패");
    size_t pos = 0;
    std::string word;

    while (pos != std::string::npos && pos < str.size()) {
        word = getWord(str, pos);
        if (word == "")
            break;
        values.push_back(word);
    }
    i = end + 1;
    return values;
}

std::string getValue(std::string const &fileContent, size_t &i, char delimiter) {
    std::vector<std::string> values = getValues(fileContent, i, delimiter);
    if (values.size() != 1)
        throw std::runtime_error("config 파일 파싱 중 에러 발생: getValue실패");
    return values[0];
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
    std::cout << "[parseServer] " << "globalPortList: ";
    for (std::vector<int>::iterator it = ServerConfig::globalPortList.begin(); it != ServerConfig::globalPortList.end(); it++)
        std::cout << *it << " ";
    std::cout << std::endl;
    std::cout << "[parseServer] " << "ip: " << server.getIp().s_addr << std::endl;
    std::cout << "[parseServer] " << "port: " << server.getPort() << std::endl;
    std::cout << "[parseServer] " << "serverName: " << server.getServerName() << std::endl;
}

std::vector<std::pair<std::string, std::string> > &parseQuery(const std::string &query)
{
    std::vector<std::pair<std::string, std::string> > queryV;
    std::string key, value;
    size_t equalPos, amperPos, start = 0;

    while (start != query.length()) {
        if ((equalPos = query.find('=', start + 1)) == std::string::npos) {
            queryV.clear();
            return queryV;
        }

        if ((amperPos = query.find('&', equalPos + 1)) == std::string::npos)
            amperPos = query.length();

        if (start == 0)
            --start;
        key = query.substr(start + 1, equalPos - start - 1);
        value = query.substr(equalPos + 1, amperPos - equalPos - 1);
        queryV.push_back(std::make_pair(key, value));

        start = amperPos;
    }

    return queryV;
}

std::vector<std::string> &splitByComma(const std::string &str)
{
    std::vector<std::string> ret;
    std::string token;
    std::istringstream iss(str);

    while (std::getline(iss, token, ',')) {
        if (token[0] == ' ')
            ret.push_back(token.substr(1));
        else
            ret.push_back(token);
    }
    return ret;
}

std::string &decodeUrl(const std::string &str)
{
    std::ostringstream decoded;
    std::string res;

    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '%') {
            if (i + 2 < str.length() && isxdigit(str[i + 1]) && isxdigit(str[i + 2])) {
                char decodedChar = static_cast<char>(strtol(str.substr(i + 1, 2).c_str(), nullptr, 16));
                decoded << decodedChar;
                i += 2;
            }
            else
                decoded << str[i];
        }
        else
            decoded << str[i];
    }

    res = decoded.str();
    return res;
}