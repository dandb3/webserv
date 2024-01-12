#include <queue>
#include "parse.hpp"
#include "CgiResponseModule.hpp"

static void readLines(const std::string& raw, std::queue<std::string>& lineQ, std::string& messageBody)
{
    size_t start = 0, end;
    bool error = true;

    while ((end = raw.find(CRLF)) != std::string::npos) {
        lineQ.push(raw.substr(start, end - start));
        start = end + 2;
        if (lineQ.back().size() == 0) {
            error = false;
            break;
        }
    }
    if (error)
        throw error;
    messageBody = raw.substr(start);
}

static std::pair<std::string, std::string> parseLine(const std::string& line)
{
    size_t pos;

    if ((pos = line.find(':')) == std::string::npos)
        throw 4400404; // ERROR
    return std::make_pair(line.substr(0, pos), line.substr(pos + 1));
}

void parseCgiResponse(CgiResponse& cgiResponse, const std::string& raw)
{
    std::queue<std::string> lineQ;
    std::string messageBody;
    std::pair<std::string, std::string> p;

    readLines(raw, lineQ, messageBody);
    parseLine(lineQ.front());
    lineQ.pop();
    if (isCaseInsensitiveSame(p.first, "Content-Type"))
        parseDocumentResponse(cgiResponse, lineQ, messageBody, p);
    else if (isCaseInsensitiveSame(p.first, "Location")) {
        
    }
}
