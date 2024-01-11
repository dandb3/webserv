#include "ConfigParser.hpp"

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
void ConfigParser::parse(std::string const &config_path, Config &config) {
    std::cout << "read config file" << std::endl;
    std::string file_content = FileReader::read_file(config_path); // 실패 시 throw
    std::cout << "finish read config file" << std::endl;
    size_t i = 0;
    std::string key;
    while (i != std::string::npos && i < file_content.size()) {
        key = getKey(file_content, i);
        if (key == "server") {
            std::cout << "parse server" << std::endl;
            parseServer(file_content, i, config);
        }
        else if (key == "include") { // include mime.types;
            std::string file_path = getValue(file_content, i, ';')[0]; // 쫌 별로네..getValue가 2개 이상이면 에러 처리?
            if (i == std::string::npos)
                throw std::runtime_error("config 파일 파싱 중 에러 발생");
            parseTypes(file_path, config);
        }
        else {
            std::vector<std::string> value = getValue(file_content, i, ';');
            config.setVariable(key, value);
        }
        i = file_content.find_first_not_of(WHITESPACE, i);
    }
}

void ConfigParser::parseServer(std::string const &file_content, size_t &i, Config &config) {
    ServerConfig server_config;
    if (getWord(file_content, i) != "{")
        throw std::runtime_error("config 파일 파싱 중 에러 발생");
    std::string key;
    while (file_content[i] != '}') {
        key = getKey(file_content, i);
        std::cout << "key: " << key << std::endl;
        if (i == std::string::npos) // 서버 파싱 전에 끝까지 파싱한 경우
            throw std::runtime_error("config 파일 파싱 중 에러 발생");
        if (key == "location") {
            std::cout << "parse location" << std::endl;
            LocationConfig location_config;
            parseLocation(file_content, i, server_config, location_config);
        }
        else {
            std::vector<std::string> value = getValue(file_content, i, ';');
            if (key == "listen") {
                if (value.size() != 1) // 만약 value[1]이 존재하면 에러 처리?
                    throw std::runtime_error("config 파일 파싱 중 에러 발생: listen은 하나만 존재해야 합니다.");
                std::pair<struct in_addr, int> ip_port = getIpPort(value[0]);
                if (ip_port.first.s_addr == INADDR_ANY) {
                    server_config.addPort(ip_port.second);
                }
                server_config.setIp(ip_port.first);
                server_config.setPort(ip_port.second);
            }
            else if (key == "server_name") {
                server_config.setServerName(value);
            }
            else
                server_config.setVariable(key, value);
        }
        i = file_content.find_first_not_of(WHITESPACE, i);
        std::cout << "file_content[i]: " << file_content[i] << std::endl;
    }
    i++;
    config.setServer(server_config);
}

// ServeraConfig에 location path 같은게 있으면 뒤에꺼는 무시하고 앞에꺼만 적용되는데
// locationConfig 구조 변경 -> path를 key로 하는 map으로 변경
void ConfigParser::parseLocation(std::string const &file_content, size_t &i, ServerConfig &server_config, LocationConfig &location_config) {
    std::map<std::string, LocationConfig> location_m = server_config.getLocationList();
    std::string path = getValue(file_content, i, '{')[0];
    if (i == std::string::npos)
        throw std::runtime_error("config 파일 파싱 중 에러 발생");
    while (file_content[i] != '}') {
        std::string key = getKey(file_content, i);
        if (i == std::string::npos)
            throw std::runtime_error("config 파일 파싱 중 에러 발생");
        if (key == "location") { // location에 대한 파싱이 다 끝난 후에야 옴.
            std::cout << "parse location" << std::endl;
            LocationConfig new_location_config = location_config;
            parseLocation(file_content, i, server_config, new_location_config);

        }
        else {
            std::vector<std::string> value = getValue(file_content, i, ';');
            location_config.setVariable(key, value);
        }
        i = file_content.find_first_not_of(WHITESPACE, i);
    }
    if (location_m.find(path) == location_m.end()) {
        server_config.setLocation(path, location_config);
    }
    i++;
}

void ConfigParser::parseTypes(std::string const &file_path, Config &config) {
    t_directives mime_types;
    std::string file_content = FileReader::read_file(file_path); // 실패 시 throw
    size_t i = 0;
    std::string key = getKey(file_content, i);
    while (i != std::string::npos && i < file_content.size()) {
        std::vector<std::string> value = getValue(file_content, i, ';');
        mime_types[key] = value;
        key = getWord(file_content, i);
    }
    config.setMimeTypes(mime_types);
}
