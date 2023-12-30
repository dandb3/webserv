#include "ConfigParser.hpp"

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
