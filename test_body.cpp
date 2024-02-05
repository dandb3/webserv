#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <sstream>
#include <set>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/event.h>
#include <sys/wait.h>
#include <queue>
#include <fstream>
#define CRLF "\r\n"
using namespace std;

std::string message_body, _remain;

size_t len;

void _inputDefaultBody(int contentLengthCount, int transferEncodingCount);
void _inputChunkedBody(int transferEncodingCount);

std::vector<std::string> _splitByComma(std::string &str)
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

void _inputMessageBody(int a, int b)
{
    int contentLengthCount = a;
    int transferEncodingCount = b;

    if (contentLengthCount > 0)
        _inputDefaultBody(contentLengthCount, transferEncodingCount);
    else if (transferEncodingCount > 0)
        _inputChunkedBody(transferEncodingCount);
}

void _inputDefaultBody(int contentLengthCount, int transferEncodingCount)
{
    std::string lengthStr;
    long long length;

    if (transferEncodingCount > 0) // sender MUST NOT
        return;
    if (contentLengthCount == 1)
        lengthStr = to_string(len);
    else {
        lengthStr = to_string(len);
        // std::multimap<std::string, std::string> &mp = _httpRequest.getHeaderFields();
        // std::multimap<std::string, std::string>::iterator iter = mp.find("Content-Length");

        // lengthStr = iter->second;
        // for (iter++; iter != mp.end() && iter->first == "Content-Length"; iter++) {
        //     if (lengthStr != iter->second) // 404 error
        //         return;
        // }
    }

    std::vector<std::string> lengthV = _splitByComma(lengthStr);
    if (lengthV.size() != 1) {
        for (size_t i = 1; i < lengthV.size(); i++) {
            if (lengthV[0] != lengthV[i]) // 404 error
                return;
        }
        lengthStr = lengthV[0];
    }
    length = strtol(lengthStr.c_str(), NULL, 10);

    message_body.append(_remain.substr(0, length));
}

void _inputChunkedBody(int transferEncodingCount)
{
    enum {
        LENGTH = 0,
        STRING
    };

    size_t start, end;
    long long length;
    short mode = LENGTH;

    (void) transferEncodingCount;
    start = 0;
    while (1) {
        end = _remain.find(CRLF, start);
        if (end == std::string::npos) {
            if (_remain[start] == '0' && _remain[start + 1] == '\0') {
                cout << "successfully finished" << endl;
                break;
            }
            else {
                // 400 Bad Request
                cout << "error\n";
                break;
            }
        }
            
        if (mode == LENGTH) {
            length = strtol(_remain.substr(start, end - start).c_str(), NULL, 10);
            mode = STRING;
            cout << "length: " << length << endl;
        }
        else {
            message_body.append(_remain.substr(start, length));
            mode = LENGTH;
        }
        start = end + 2;
    }
}

int main(int ac, char **av)
{
    if (ac != 3) {
        std::cerr << "Usage: " << av[0] << ": content-length, transfer-encoding" << std::endl;
        exit(1);
    }

    char buf[1024];
    int fd = open("test.txt", O_RDONLY);
    memset(buf, 0, 1024);
    if (read(fd, buf, 1024) == -1)
        std::cerr << "read error" << std::endl;
    
    size_t len = strlen(buf);
    (void) len;
    _remain = buf;
    _remain.append("\0");

    _inputMessageBody(atoi(av[1]), atoi(av[2]));
    cout << message_body << endl;
    return 0;
}