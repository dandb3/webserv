#include "ConfigParser.hpp"

int ft_inet_aton(const char* str, struct in_addr* addr) {
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

std::pair<struct in_addr, int> ConfigParser::getIpPort(std::string listen) {
    std::string ip_str;
    struct in_addr ip;
    int port;
    size_t colon = listen.find(":");
    if (colon == std::string::npos) {
        if (listen.find(".") == std::string::npos) { // port만 있는 경우
            ip_str = "0.0.0.0";
            port = atoi(listen.c_str());
        }
        else { // ip만 있는 경우
            ip_str = listen;
            port = 8080;
        }
    }
    else { // ip:port 형식인 경우
        ip_str = listen.substr(0, listen.find(":"));
        port = atoi(listen.substr(listen.find(":") + 1).c_str());
    }
    if (ft_inet_aton(ip_str.c_str(), &ip) == 0) {
        throw std::runtime_error("ip 주소 변환 실패");
    }
    return std::make_pair(ip, port);
}

std::string ConfigParser::getWord(std::string const& file_content, size_t& i, std::string const& delimiter) {
    size_t start = file_content.find_first_not_of(delimiter, i);
    size_t end = file_content.find_first_of(delimiter, start);
    if (start == std::string::npos || end == std::string::npos) {
        i = std::string::npos; // 파일 끝 표시 대입
        return ""; // 더 이상 읽을 내용이 없음을 나타냄
    }
    // 다음 오는 문자가 ';' 인 경우(해당 라인의 끝인 경우)
    size_t next = file_content.find_first_not_of(WHITESPACE, end);
    if (next != std::string::npos && file_content[next] == ';')
        i = next;
    else
        i = end;
    return file_content.substr(start, end - start);
}

/**
 * @brief config 파일을 파싱하는 함수
 * 1. 파일을 읽어서 string으로 반환
 * 2. string 파싱
 * 2-1. 첫 단어를 key로, key가 server일 경우 parseServer() 호출
 *  2-1-1. server_info에 parseServer()에서 구한 server 추가
 * 2-2. key가 include일 경우 mime.types 파일을 읽어서 Config에 저장
 * 2-3. 그 외의 경우 ; 전까지의 내용을 value로 저장
 *
 * 파싱 시에 key 값이 내가 찾는 값이 아닌경우 key가 무엇인지 알려주고 오류 처리 (해야할 것)
*/
void ConfigParser::parse(std::string const& config_path, Config& config) {
    std::cout << "read config file" << std::endl;
    std::string file_content = FileReader::read_file(config_path); // 실패 시 throw
    std::cout << "finish read config file" << std::endl;
    size_t i = 0;
    std::string key = getWord(file_content, i, DELIMITER);
    while (i != std::string::npos && i < file_content.size()) {
        if (key == "server") {
            parseServer(file_content, i, config);
        }
        else if (key == "include") { // include mime.types;
            std::string file_path = getWord(file_content, i, DELIMITER);
            if (i == std::string::npos)
                throw std::runtime_error("config 파일 파싱 중 에러 발생");
            parseTypes(file_path, config);
        }
        else {
            std::vector<std::string> value;
            while (file_content[i] != ';') {
                std::string word = getWord(file_content, i, DELIMITER);
                if (i == std::string::npos)
                    throw std::runtime_error("config 파일 파싱 중 에러 발생");
                value.push_back(word);
            }
            config.setVariable(key, value);
        }
        key = getWord(file_content, i, DELIMITER);
    }

}

void ConfigParser::parseServer(std::string const& file_content, size_t& i, Config& config) {
    ServerConfig server_config;
    if (getWord(file_content, i, DELIMITER) != "{")
        throw std::runtime_error("config 파일 파싱 중 에러 발생");
    std::string key = getWord(file_content, i, DELIMITER);
    if (i == std::string::npos)
        throw std::runtime_error("config 파일 파싱 중 에러 발생");
    while (key != "}") {
        if (key == "location") {
            parseLocation(file_content, i, server_config);
        }
        else {
            std::vector<std::string> value;
            while (file_content[i] != ';') {
                std::string word = getWord(file_content, i, DELIMITER);
                if (i == std::string::npos)
                    throw std::runtime_error("config 파일 파싱 중 에러 발생");
                value.push_back(word);
            }
            if (key == "listen") {
                std::pair<struct in_addr, int> ip_port = getIpPort(value[0]);
                if (ip_port.first.s_addr == INADDR_ANY)
                    server_config.portsWithINADDR_ANY.push_back(ip_port.second);
                server_config.setIp(ip_port.first);
                server_config.setPort(ip_port.second);
            }
            else if (key == "server_name") {
                server_config.setServerName(value);
            }
            else
                server_config.setVariable(key, value);
        }
        key = getWord(file_content, i, DELIMITER);
        if (i == std::string::npos)
            throw std::runtime_error("config 파일 파싱 중 에러 발생");
    }
    config.setServer(server_config);
}

void ConfigParser::parseLocation(std::string const& file_content, size_t& i, ServerConfig& server_config) {
    LocationConfig location_config;
    size_t start = file_content.find_first_of('{', i);
    std::string argv;
    std::string tmp = getWord(file_content, i, DELIMITER);
    while (tmp != "{") {
        if (i >= start) // {가 단어와 붙어있는 경우
            throw std::runtime_error("config 파일 파싱 중 에러 발생");
        argv = tmp;
        tmp = getWord(file_content, i, DELIMITER);
    }
    location_config.setPath(argv);
    std::string key = getWord(file_content, i, DELIMITER);
    if (i == std::string::npos)
        throw std::runtime_error("config 파일 파싱 중 에러 발생");
    while (key != "}") {
        if (key == "location") {
            parseLocation(file_content, i, server_config);
        }
        else {
            std::vector<std::string> value;
            while (file_content[i] != ';') {
                std::string word = getWord(file_content, i, DELIMITER);
                if (i == std::string::npos)
                    throw std::runtime_error("config 파일 파싱 중 에러 발생");
                value.push_back(word);
            }
            location_config.setVariable(key, value);
        }
        key = getWord(file_content, i, DELIMITER);
        if (i == std::string::npos)
            throw std::runtime_error("config 파일 파싱 중 에러 발생");
    }
    server_config.setLocation(location_config);
}

void ConfigParser::parseTypes(std::string const& file_path, Config& config) {
    t_directives mime_types;
    std::string file_content = FileReader::read_file(file_path); // 실패 시 throw
    size_t i = 0;
    std::string key = getWord(file_content, i, DELIMITER);
    while (i != std::string::npos && i < file_content.size()) {
        std::vector<std::string> value;
        while (file_content[i] != ';') {
            std::string word = getWord(file_content, i, DELIMITER);
            if (i == std::string::npos)
                throw std::runtime_error("config 파일 파싱 중 에러 발생");
            value.push_back(word);
        }
        mime_types[key] = value;
        key = getWord(file_content, i, DELIMITER);
    }
    config.setMimeTypes(mime_types);
}
