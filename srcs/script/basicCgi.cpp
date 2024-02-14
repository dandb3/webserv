#include <iostream>
#include <map>
#include <string>

std::string getReasonPhrase(unsigned short code)
{
    switch (code) {
    case 200:
        return "OK";
    }
}

void printStatusHeader(unsigned short code)
{
    std::cout << "Status: " << code << getReasonPhrase(code) << "\r\n";
}

void printContentTypeHeader()
{
    std::cout << "Content-Type: text/html; charset=utf-8\r\n";
}

int main(int argc, char* argv[], char* envp[])
{
    char** envEnd = envp;
    while (*envEnd != NULL)
        ++envEnd;

    const std::map<std::string, std::string> metaVariables(envp, envEnd);

    REQUEST_METHOD
    SERVER_PROTOCOL
    PATH_INFO

    return 0;
}
