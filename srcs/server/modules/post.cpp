#include <cstring>
#include "HttpResponseModule.hpp"
#include "../../utils/utils.hpp"

bool checkString(const std::string &str, const std::string &target, const size_t &start)
{
    const size_t len = target.length();
    if (str.length() < start + len)
        return false;
    for (size_t i = 0; i < len; i++) {
        if (str[start + i] != target[i])
            return false;
    }
    return true;
}

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

void parseContentDisposition(std::map<std::string, std::string> &params, const std::string &body, size_t &start)
{
    std::vector<std::string> contents = splitByDlm(body.substr(start), ';');

    for (std::vector<std::string>::iterator it = contents.begin(); it != contents.end(); it++) {
        if (checkString(*it, "Content-Disposition", 0)) {
            size_t colonPos = it->find(':');
            if (colonPos == std::string::npos)
                throw 400;
            if (it->at(colonPos + 1) == ' ')
                ++colonPos;
            if (it->substr(colonPos) != "form-data")
                throw 400;
        }
        else if (checkString(*it, "name=", 0)) {
            std::string val = it->substr(6);
            val.pop_back();
            params.insert(std::make_pair("name", val));
        }
        else if (checkString(*it, "filename=", 0)) {
            std::string val = it->substr(10);
            val.pop_back();
            params.insert(std::make_pair("filename", val));
        }
    }    
}

void parseContentType(const std::string &body, size_t &curPos, std::string &boundaryContentType)
{
    size_t colonPos = body.find(':', curPos);
    const size_t crlfPos = body.find(CRLF, curPos);
    if (colonPos == std::string::npos)
        throw 400;
    if (body[colonPos + 1] == ' ')
        ++colonPos;
    
    boundaryContentType = body.substr(colonPos + 1, crlfPos - colonPos - 1);
}

void parseMultiForm(const std::string &contentType, const std::string &body, std::map<std::string, std::string> &files)
{
    static int fileIndex = 1;
    const size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos)
        throw 400;
    const std::string boundary = "--" + contentType.substr(pos + 9);
    
    size_t curPos, endPos = body.find(boundary);
    if (endPos == std::string::npos)
        return;

    while (1) {
        bool content = false;
        std::map<std::string, std::string> params;
        std::string boundaryContentType;

        curPos = body.find(CRLF, endPos) + 2;
        endPos = body.find(boundary, curPos);

        while (1) {
            if (content) {
                std::string fileContent = body.substr(curPos, endPos - curPos - 1), fileName;
                std::map<std::string, std::string>::iterator it = params.find("filename");
                if (it == params.end())
                    fileName = "file" + toString(fileIndex++);
                else
                    fileName = it->second;

                if (boundaryContentType == "application/x-www-form-urlencoded")
                    std::string fileContent = parseUrlencode(fileContent);
                else if (boundaryContentType == "text/plain")
                    parseTextPlain(fileContent);
                files.insert(std::make_pair(fileName, fileContent));
                
                break;
            }
            if (checkString(body, "Content-Disposition", curPos)) {
                parseContentDisposition(params, body, curPos);
            }
            else if (checkString(body, "Content-Type", curPos)) {
                parseContentType(body, curPos, boundaryContentType);
            }
            else if (body[curPos] == '\r' && body[curPos + 1] == '\n') {
                content = true;
            }
            curPos = body.find(CRLF, curPos) + 2;
        }
    }
}

void HttpResponseHandler::_makePOSTResponse(HttpRequest &httpRequest, ConfigInfo &configInfo)
{
    std::map<std::string, std::string> files;
    const std::string contentType = httpRequest.getHeaderFields().find("Content-Type")->second;
    const std::string body = httpRequest.getMessageBody();
    const std::string fileName = httpRequest.getRequestLine().getUri();
    std::string fileContent;

    if (contentType == "") {
        if (body == "") // 실제 body가 없는 경우
            _makeStatusLine(204);
        else { // 실제 body가 있는 경우 -> binary로 해석
            // _makeStatusLine(400);
        }
    }

    if (checkString(contentType, "multipart/form-data", 0)) {
        parseMultiForm(contentType, body, files);
    }
    else {
        if (checkString(contentType, "application/x-www-form-urlencoded", 0))
            fileContent = parseUrlencode(body);
        else if (checkString(contentType, "text/plain", 0)) {
            fileContent = body;
            parseTextPlain(fileContent);
        }
        else
            fileContent = body;
        files.insert(std::make_pair(fileName, fileContent));
    }
}
