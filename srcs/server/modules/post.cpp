#include <cstring>
#include <sstream>
#include "../../utils/utils.hpp"
#include "post.hpp"

std::string parseUrlencode(const std::string &encodedUrl)
{
    std::vector<std::pair<std::string, std::string> > query;
    std::string decodedUrl = decodeUrl(encodedUrl);
    query = parseQuery(decodedUrl);

    std::stringstream resultStream;

    for (std::vector<std::pair<std::string, std::string> >::iterator it = query.begin(); it != query.end();) {
        resultStream << it->first << ": " << it->second;
        if (++it != query.end()) {
            resultStream << "\r\n";
        }
    }

    return resultStream.str();
}

void parseTextPlain(std::string &body)
{
    for (size_t i = 0; i < body.length(); i++) {
        if (body[i] == '+')
            body[i] = ' ';
    }
}

static void parseContentDisposition(std::map<std::string, std::string> &params, const std::string &body, size_t &start)
{
    std::string dispositionLine = body.substr(start, body.find(CRLF, start) - start);
    std::vector<std::string> contents = splitByDlm(dispositionLine, ';');

    for (std::vector<std::string>::iterator it = contents.begin(); it != contents.end(); it++) {
        if (checkString(*it, "Content-Disposition", 0)) {
            size_t colonPos = it->find(':');
            if (colonPos == std::string::npos)
                throw 400;
            if (it->at(colonPos + 1) == ' ')
                ++colonPos;
            if (it->substr(colonPos + 1) != "form-data")
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

static void parseContentType(const std::string &body, size_t &curPos, std::string &boundaryContentType)
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

        if (curPos == 1)
            break;

        while (1) {
            if (content) {
                std::string fileContent = body.substr(curPos, endPos - curPos - 1), fileName;
                std::map<std::string, std::string>::iterator it = params.find("filename");

                if (it == params.end())
                    fileName = "file" + toString(fileIndex++);
                else
                    fileName = it->second;

                if (boundaryContentType == "application/x-www-form-urlencoded") {
                    fileContent = parseUrlencode(fileContent);
                    parseTextPlain(fileContent);
                }
                else if (boundaryContentType == "text/plain") {
                    parseTextPlain(fileContent);
                }
                if (files.find(fileName) == files.end())
                    throw 409;
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
