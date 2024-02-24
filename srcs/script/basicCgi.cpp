#include <iostream>
#include <vector>
#include <map>
#include <string>

std::string getReasonPhrase(unsigned short code)
{
    switch (code) {
    case 200:
        return "OK";
    case 501:
        return "Not Implemented";
    default:
        return "";
    }
}

void printStatusHeader(unsigned short code)
{
    std::cout << "Status: " << code << ' ' << getReasonPhrase(code) << "\r\n";
}

void printContentTypeHeader()
{
    std::cout << "Content-Type: text/plain; charset=utf-8\r\n";
}

void printEmptyLine()
{
    std::cout << "\r\n";
}

void printMessageBody()
{
    std::cout <<                            \
        "    /\\     /\\\n"                 \
        "   {  `---'  }\n"                  \
        "   {  O   O  }\n"                  \
        "~~&gt;  V  &lt;~~\n"               \
        "    \\  \\|/  /\n"                 \
        "     `-----'____\n"                \
        "     /     \\    \\_\n"            \
        "    {       }\\  )_\\_   _\n"      \
        "    |  \\_/  |/ /  \\_\\_/ )\n"    \
        "     \\__/  /(_/     \\__/\n"      \
        "       (__/\n"                     \
        << std::endl;
}

int main(__attribute__((unused)) int argc, __attribute__((unused)) char* argv[], char* envp[])
{
    int envCnt = 0;
    while (envp[envCnt] != NULL)
        ++envCnt;
    std::vector<std::pair<std::string, std::string> > metaVariablesV(envCnt);
    for (int i = 0; i < envCnt; ++i) {
        std::string env = envp[i];
        size_t sep = env.find('=');

        metaVariablesV[i].first = env.substr(0, sep);
        metaVariablesV[i].second = env.substr(sep + 1);
    }
    std::map<std::string, std::string> metaVariables(metaVariablesV.begin(), metaVariablesV.end());
    std::string& requestMethod = metaVariables["REQUEST_METHOD"];

    if (requestMethod == "GET" || requestMethod == "HEAD" || requestMethod == "POST" || requestMethod == "DELETE") {
        printStatusHeader(200);
        printContentTypeHeader();
        printEmptyLine();
        printMessageBody();
    }
    else {
        printStatusHeader(501);
        printEmptyLine();
    }
    return 0;
}
