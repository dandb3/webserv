#include <cstring>
#include "HttpResponseModule.hpp"
#include "../../utils/utils.hpp"

// 최적화를 위하여 string 대신 vector<char> 이용함
std::string &parseUrlencode(const std::string &encodedUrl)
{
    std::vector<std::pair<std::string, std::string> > query;
    std::vector<char> resultVec;
    std::string decodedUrl = decodeUrl(encodedUrl);
    query = parseQuery(decodedUrl);

    for (std::vector<std::pair<std::string, std::string> >::iterator it = query.begin(); it != query.end(); it++) {
        for (size_t i = 0; i < it->first.length(); i++)
            resultVec.push_back(it->first[i]);
        resultVec.push_back(':');
        resultVec.push_back(' ');
        for (size_t i = 0; i < it->second.length(); i++)
            resultVec.push_back(it->second[i]);
        resultVec.push_back('\r');
        resultVec.push_back('\n');
    }

    std::string resultString(resultVec.data(), resultVec.size());
    return resultString;
}

void parseTextPlain(std::string &body)
{
    for (size_t i = 0; i < body.length(); i++) {
        if (body[i] == '+')
            body[i] = ' ';
    }
}

std::string &getBoundary(const std::string &contentType)
{
    const size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos)
        throw 400;
    std::string boundary = contentType.substr(pos + 9);

    return boundary;
}

void parseContentDisposition(std::map<std::string, std::string> &params, const std::string &body, size_t &start)
{
    std::vector<std::string> contents = splitByDlm(body.substr(start), ';');

    for (std::vector<std::string>::iterator it = contents.begin(); it != contents.end(); it++) {
        if (std::strncmp(it->c_str(), "Content-Disposition", 19) == 0) {
            size_t colonPos = it->find(':');
            if (colonPos == std::string::npos)
                throw 400;
            if (it->at(colonPos + 1) == ' ')
                ++colonPos;
            if (it->substr(colonPos) != "form-data")
                throw 400;
        }
        else if (std::strncmp(it->c_str(), "name=", 5) == 0) {
            std::string val = it->substr(6);
            val.pop_back();
            params.insert(std::make_pair("name", val));
        }
        else if (std::strncmp(it->c_str(), "filename=", 9) == 0) {
            std::string val = it->substr(10);
            val.pop_back();
            params.insert(std::make_pair("filename", val));
        }
    }    
}

void parseMultiForm(const std::string &contentType, const std::string &body)
{
    const size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos)
        throw 400;
    const std::string boundary = "--" + getBoundary(contentType);
    
    size_t curPos, nxtPos = body.find(boundary);

    while (1) {
        std::map<std::string, std::string> params;
        std::string boundaryContentType;

        curPos = body.find(CRLF, nxtPos) + 2;
        nxtPos = body.find(boundary, curPos);

        while (1) {
            if (body[curPos] == '\r' && body[curPos + 1] == '\n') {
                curPos += 2;
                break;
            }
            if (std::strncmp("Content-Disposition", body.substr(curPos, 19).c_str(), 19) == 0) {
                parseContentDisposition(params, body, curPos);
            }
        }
    }
}

void HttpResponseHandler::_makePOSTResponse(HttpRequest &httpRequest, ConfigInfo &configInfo)
{
    std::map<std::string, std::string> files;
    const std::string contentType = httpRequest.getHeaderFields().find("Content-Type")->second;
    const std::string fileName = httpRequest.getRequestLine().getUri();
    std::string fileContent;

    if (contentType == "") {
        if (httpRequest.getMessageBody() == "") // 실제 body가 없는 경우
            _makeStatusLine(204);
        else { // 실제 body가 있는 경우 -> binary로 해석
            // _makeStatusLine(400);
        }
    }
    else if (std::strncmp(contentType.c_str(), "application/x-www-form-urlencoded", 33) == 0) {
        fileContent = parseUrlencode(httpRequest.getMessageBody());
        files.insert(std::make_pair(fileName, fileContent));
    }
    else if (std::strncmp(contentType.c_str(), "text/plain", 10) == 0) {
        fileContent = httpRequest.getMessageBody();
        parseTextPlain(fileContent);
        files.insert(std::make_pair(fileName, fileContent));
    }
    else if (std::strncmp(contentType.c_str(), "multipart/form-data", 19) == 0) {
        parseMultiForm(contentType, httpRequest.getMessageBody());
    }
    else { // binary로 해석
        files.insert(std::make_pair(fileName, httpRequest.getMessageBody()));
    }
}