#include <vector>
#include "parse.hpp"
#include "CgiResponseModule.hpp"

static void readLines(const std::string& raw, std::vector<std::string>& lineV, std::string& messageBody)
{
    size_t start = 0, end;
    bool error = true;

    while ((end = raw.find(CRLF)) != std::string::npos) {
        lineV.push_back(raw.substr(start, end - start));
        start = end + 2;
        if (lineV.back().size() == 0) {
            error = false;
            break;
        }
    }
    if (error)
        throw error;
    messageBody = raw.substr(start);
}

static pair_t parseLine(const std::string& line)
{
    size_t fieldNameEnd, fieldValueStart;

    if ((fieldNameEnd = line.find(':')) == std::string::npos)
        throw 4400404; // ERROR
    fieldValueStart = fieldNameEnd + 1;
    eatOWS(line, fieldValueStart);
    return std::make_pair(line.substr(0, fieldNameEnd), line.substr(fieldValueStart));
}

bool isContentType(const pair_t& p)
{
    return (isCaseInsensitiveSame(p.first, "Content-Type") \
        && isMediaType(p.second));
}

void parseCgiResponse(CgiResponse& cgiResponse, const std::string& raw, char& type)
{
    std::vector<std::string> lineV;
    std::vector<pair_t> pairV;
    std::string messageBody;

    readLines(raw, lineV, messageBody);
    for (size_t i = 0; i + 1 < lineV.size(); ++i)
        pairV.push_back(parseLine(lineV[i]));
    if (pairV.empty())
        throw 501; // ERROR;
    if (isContentType(pairV[0])) {
        parseDocumentResponse(cgiResponse, pairV, messageBody);
        type = CgiResponseHandler::DOCUMENT_RES;
    }
    else if (isLocalLocation(pairV[0])) {
        parseLocalRedirResponse(cgiResponse, pairV, messageBody);
        type = CgiResponseHandler::LOCAL_REDIR_RES;
    }
    else if (pairV.size() == 1) {
        parseClientRedirResponse(cgiResponse, pairV, messageBody);
        type = CgiResponseHandler::CLIENT_REDIR_RES;
    }
    else {
        parseClientRedirdocResponse(cgiResponse, pairV, messageBody);
        type = CgiResponseHandler::CLIENT_REDIR_DOC_RES;
    }
}
