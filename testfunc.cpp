#include <iostream>
#include <ctime>
#include <iomanip>
#include <sys/stat.h>

void getFileTime()
{
    const char* filename = "/goinfre/sunwsong/webserv/test2.txt";

    struct stat fileInfo;
    
    if (stat(filename, &fileInfo) != -1) {
        std::time_t lastModifiedTime = fileInfo.st_mtime;
        
        // 시간 구조체로 변환
        std::tm* timeInfo = std::gmtime(&lastModifiedTime);

        // RFC 7231 형식으로 날짜를 문자열로 변환하기 위한 버퍼
        char buffer[80];

        // 시간을 형식에 맞게 문자열로 변환
        std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);

        // Last-Modified 헤더 출력
        std::cout << "Last-Modified: " << buffer << std::endl;
    }
    else {
        std::cerr << "Error accessing file information." << std::endl;
    }
    
}

void getCurrentTime()
{
    // 현재 시간 얻기
    std::time_t currentTime = std::time(nullptr);

    // 시간 구조체로 변환
    std::tm* timeInfo = std::gmtime(&currentTime);

    // RFC 7231 형식으로 날짜를 문자열로 변환하기 위한 버퍼
    char buffer[80];

    // 시간을 형식에 맞게 문자열로 변환
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);

    // 날짜 및 시간 출력
    std::cout << buffer << std::endl;

}

int main() {
    getFileTime();
    getCurrentTime();
    return 0;
}
