#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>
#include <cctype>
#include <map>
#include <cstdlib>
using namespace std;

string decodeUrl(string &str)
{
    ostringstream decoded;

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
        else if (str[i] == '+')
            decoded << ' ';
        else
            decoded << str[i];
    }

    return decoded.str();
}

void parseQuery(std::string &query)
{
    std::vector<std::pair<std::string, std::string> > queryV;
    std::string key, value;
    size_t equalPos, amperPos, start = 0;

    while (start != query.length()) {
        if ((equalPos = query.find('=', start + 1)) == std::string::npos)
            return; // GET 요청에 문법 오류(error 발생)

        if ((amperPos = query.find('&', equalPos + 1)) == std::string::npos)
            amperPos = query.length();

        if (start == 0)
            --start;
        key = query.substr(start + 1, equalPos - start - 1);
        value = query.substr(equalPos + 1, amperPos - equalPos - 1);
        queryV.push_back(std::make_pair(key, value));

        start = amperPos;
    }
}

int main(int ac, char **av)
{
    (void) ac;
    string a = av[1];
    string token = decodeUrl(a);
    cout << "token: " << token << endl;
    string uri;
    std::vector<std::pair<std::string, std::string> > queryV;
    size_t pos = token.find('?');
    if (pos == std::string::npos) 
        uri = token;
    else {
        std::string urii = token.substr(0, pos);
        std::string queries = token.substr(pos + 1);
        uri = urii;
        parseQuery(queries);
    }
    std::cout << uri << std::endl;
    return 0;
}