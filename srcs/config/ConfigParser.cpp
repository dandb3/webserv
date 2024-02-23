#include "ConfigParser.hpp"

// const std::set<std::string> ConfigParser::SERVER_KEY_SET = {
//     "listen", "server_name", "root", "index", "autoindex", "client_max_body_size",
//     "error_page", "location", "access_log", "error_log", "cgi", "allow_methods",
//     "request_timeout", "keepalive_timeout", "session" };

std::set<std::string> init() {
    std::set<std::string> s;
    s.insert("listen");
    s.insert("server_name");
    s.insert("root");
    s.insert("index");
    s.insert("autoindex");
    s.insert("limit_client_body_size");
    s.insert("error_page");
    s.insert("location");
    s.insert("access_log");
    s.insert("error_log");
    s.insert("return");
    s.insert("cgi");
    s.insert("cgi_path");
    s.insert("allow_method");
    s.insert("request_timeout");
    s.insert("keepalive_timeout");
    s.insert("session");
    return s;
}

const std::set<std::string> ConfigParser::SERVER_KEY_SET = init();

std::pair<struct in_addr, int> ConfigParser::getIpPort(std::string listen) {
    std::string ipStr;
    struct in_addr ip;
    int port;
    size_t colon = listen.find(":");
    if (colon == std::string::npos) {
        if (listen.find(".") == std::string::npos) { // port만 있는 경우
            ipStr = "0.0.0.0";
            port = atoi(listen.c_str());
        }
        else { // ip만 있는 경우
            ipStr = listen;
            port = 8080;
        }
    }
    else { // ip:port 형식인 경우
        ipStr = listen.substr(0, listen.find(":"));
        port = atoi(listen.substr(listen.find(":") + 1).c_str());
    }
    if (ft_inet_aton(ipStr.c_str(), &ip) == 0) {
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
void ConfigParser::parse(std::string const &configPath, Config &config) {
    std::string fileContent = FileReader::read_file(configPath); // 실패 시 throw
    size_t i = 0;
    std::string key;
    while (i != std::string::npos && i < fileContent.size()) {
        key = getKey(fileContent, i);
        if (key == "server") {
            parseServer(fileContent, i, config);
        }
        else if (key == "include") { // include mime.types;
            std::string filePath = getValue(fileContent, i, ';');
            if (i == std::string::npos)
                throw std::runtime_error("config 파일 파싱 중 에러 발생");
            parseTypes(filePath, config);
        }
        else {
            std::vector<std::string> value = getValues(fileContent, i, ';');
            config.setVariable(key, value);
        }
        i = fileContent.find_first_not_of(WHITESPACE, i);
    }
}

bool ConfigParser::isAlreadyExist(std::vector<ServerConfig> &serverList, ServerConfig &serverConfig) {
    for (std::vector<ServerConfig>::iterator serverIt = serverList.begin(); serverIt != serverList.end(); serverIt++) {
        if (serverIt->getIp().s_addr == serverConfig.getIp().s_addr && serverIt->getPort() == serverConfig.getPort() && \
            serverIt->getServerName() == serverConfig.getServerName()) {
            return true;
        }
    }
    return false;
}

void ConfigParser::parseServer(std::string const &fileContent, size_t &i, Config &config) {
    ServerConfig serverConfig;
    if (getWord(fileContent, i) != "{")
        throw std::runtime_error("config 파일 파싱 중 에러 발생");
    std::string key;
    while (fileContent[i] != '}') {
        key = getKey(fileContent, i);
        if (i == std::string::npos || ConfigParser::SERVER_KEY_SET.find(key) == ConfigParser::SERVER_KEY_SET.end())
            throw std::runtime_error("config 파일 파싱 중 에러 발생");
        if (key == "location") {
            LocationConfig locationConfig;
            parseLocation(fileContent, i, serverConfig, locationConfig);
        }
        else {
            if (key == "listen") {
                std::string value = getValue(fileContent, i, ';');

                std::pair<struct in_addr, int> ipPort = getIpPort(value);
                if (ipPort.first.s_addr == INADDR_ANY) {
                    serverConfig.addPort(ipPort.second);
                }
                serverConfig.setIp(ipPort.first);
                serverConfig.setPort(ipPort.second);
            }
            else if (key == "server_name") {
                std::string value = getValue(fileContent, i, ';');

                serverConfig.setServerName(value);
            }
            else {
                std::vector<std::string> value = getValues(fileContent, i, ';');
                serverConfig.setVariable(key, value);
            }
        }
        i = fileContent.find_first_not_of(WHITESPACE, i);
    }
    i++;
    if (!isAlreadyExist(config.getServerConfig(), serverConfig))
        config.setServer(serverConfig);
}

// ServeraConfig에 location path 같은게 있으면 뒤에꺼는 무시하고 앞에꺼만 적용되는데
// locationConfig 구조 변경 -> path를 key로 하는 map으로 변경
void ConfigParser::parseLocation(std::string const &fileContent, size_t &i, ServerConfig &serverConfig, LocationConfig &locationConfig) {
    std::map<std::string, LocationConfig> locationMap = serverConfig.getLocationList();
    std::string path = getValue(fileContent, i, '{');
    if (i == std::string::npos)
        throw std::runtime_error("config 파일 파싱 중 에러 발생");
    if (path[path.size() - 1] != '/')
        path += '/';
    while (fileContent[i] != '}') {
        std::string key = getKey(fileContent, i);
        if (i == std::string::npos)
            throw std::runtime_error("config 파일 파싱 중 에러 발생");
        if (key == "location") { // location에 대한 파싱이 다 끝난 후에야 옴.
            LocationConfig newLocationConfig = locationConfig;
            parseLocation(fileContent, i, serverConfig, newLocationConfig);
        }
        else {
            std::vector<std::string> value = getValues(fileContent, i, ';');
            locationConfig.setVariable(key, value);
        }
        i = fileContent.find_first_not_of(WHITESPACE, i);
    }
    if (locationMap.find(path) == locationMap.end()) {
        serverConfig.setLocation(path, locationConfig);
    }
    i++;
}

void ConfigParser::parseTypes(std::string const &filePath, Config &config) {
    std::map<std::string, std::string> mime_types;
    std::string fileContent = FileReader::read_file(filePath); // 실패 시 throw
    size_t i = 0;
    std::string value = getKey(fileContent, i);
    while (i != std::string::npos && i < fileContent.size()) {
        std::vector<std::string> keys = getValues(fileContent, i, ';');
        for (size_t idx = 0; idx < keys.size(); ++idx)
            mime_types[keys[idx]] = value;
        value = getWord(fileContent, i);
    }
    config.setMimeTypes(mime_types);
}
