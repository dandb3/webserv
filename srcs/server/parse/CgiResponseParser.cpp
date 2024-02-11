#include "CgiResponseParser.hpp"


static bool isContentType(const pair_t& p)
{
    return (isCaseInsensitiveSame(p.first, "Content-Type") && isMediaType(p.second));
}

static bool isLocalLocation(const pair_t& p)
{
    return (isCaseInsensitiveSame(p.first, "Location") && isLocalPathquery(p.second));
}

static bool isClientLocation(const pair_t& p)
{
    return (isCaseInsensitiveSame(p.first, "Location") && isFragmentURI(p.second));
}

static bool isStatus(const pair_t& p)
{
    const std::string& value = p.second;
    size_t pos = 0;

    if (!isCaseInsensitiveSame(p.first, "Status"))
        return false;
    if (!isStatusCode(value, pos))
        return false;
    pos += 3;
    if (pos >= value.size())
        return false;
    if (value[pos++] != ' ')
        return false;
    eatReasonPhrase(value, pos);
    if (pos < value.size())
        return false;
    return true;
}

static bool isExtension(const pair_t& p)
{
    if (p.first.find("X-CGI-") == 0)
        return true;
    return false;
}

CgiResponseParser CgiResponseParser::_cgiResponseParser;

CgiResponseParser::CgiResponseParser()
{}

CgiResponseParser::CgiResponseParser(const CgiResponseParser& cgiResponseParser)
{}

CgiResponseParser& CgiResponseParser::operator=(const CgiResponseParser& cgiResponseParser)
{
    return *this;
}

CgiResponseParser::~CgiResponseParser()
{}

CgiResponseParser& CgiResponseParser::_getInstance()
{
    return _cgiResponseParser;
}

void CgiResponseParser::_init()
{
    _lineV.clear();
    _pairV.clear();
    _messageBody.clear();
    _fieldCnt = std::vector<int>(6, 0);
}

void CgiResponseParser::_readLines(const std::string& raw)
{
    size_t start = 0, end;
    bool error = true;

    while ((end = raw.find(CRLF, start)) != std::string::npos) {
        _lineV.push_back(raw.substr(start, end - start));
        start = end + 2;
        if (_lineV.back().size() == 0) {
            error = false;
            break;
        }
    }
    if (error)
        throw 502;
    _messageBody = raw.substr(start);
}

void CgiResponseParser::_parseLines()
{
    std::string fieldName, fieldValue;
    size_t fieldNameEnd, fieldValueStart;

    for (size_t i = 0; i + 1 < _lineV.size(); ++i) {
        if (!isGenericField(_lineV[i]))
            throw 502;
        fieldNameEnd = _lineV[i].find(':');
        fieldValueStart = fieldNameEnd + 1;
        eatOWS(_lineV[i], fieldValueStart);
        fieldName = _lineV[i].substr(0, fieldNameEnd);
        fieldValue = _lineV[i].substr(fieldValueStart);
        if (!fieldValue.empty())
            _pairV.push_back(std::make_pair(_lineV[i].substr(0, fieldNameEnd), _lineV[i].substr(fieldValueStart)));
    }
    if (_pairV.empty())
        throw 502;
}

char CgiResponseParser::_determineType()
{
    for (size_t i = 0; i < _pairV.size(); ++i) {
        if (isContentType(_pairV[i]))
            ++_fieldCnt[HDR_CONTENT_TYPE];
        else if (isLocalLocation(_pairV[i]))
            ++_fieldCnt[HDR_LOCAL_LOCATION];
        else if (isClientLocation(_pairV[i]))
            ++_fieldCnt[HDR_CLIENT_LOCATION];
        else if (isStatus(_pairV[i]))
            ++_fieldCnt[HDR_STATUS];
        else if (isExtension(_pairV[i]))
            ++_fieldCnt[HDR_EXTENSION];
        else
            ++_fieldCnt[HDR_PROTOCOL];
    }
    if (_fieldCnt[HDR_CONTENT_TYPE] == 1 && _fieldCnt[HDR_LOCAL_LOCATION] == 0 \
        && _fieldCnt[HDR_CLIENT_LOCATION] == 0 && _fieldCnt[HDR_STATUS] <= 1)
        return CgiResponse::DOCUMENT_RES;
    else if (_fieldCnt[HDR_CONTENT_TYPE] == 0 && _fieldCnt[HDR_LOCAL_LOCATION] == 1 \
        && _fieldCnt[HDR_CLIENT_LOCATION] == 0 && _fieldCnt[HDR_STATUS] == 0 \
        && _fieldCnt[HDR_PROTOCOL] == 0 && _fieldCnt[HDR_EXTENSION] == 0 \
        && _messageBody.empty())
        return CgiResponse::LOCAL_REDIR_RES;
    else if (_fieldCnt[HDR_CONTENT_TYPE] == 0 && _fieldCnt[HDR_LOCAL_LOCATION] == 0 \
        && _fieldCnt[HDR_CLIENT_LOCATION] == 1 && _fieldCnt[HDR_STATUS] == 0 \
        && _fieldCnt[HDR_PROTOCOL] == 0 && _messageBody.empty())
        return CgiResponse::CLIENT_REDIR_RES;
    else if (_fieldCnt[HDR_CONTENT_TYPE] == 1 && _fieldCnt[HDR_LOCAL_LOCATION] == 0 \
        && _fieldCnt[HDR_CLIENT_LOCATION] == 1 && _fieldCnt[HDR_STATUS] == 1)
        return CgiResponse::CLIENT_REDIRDOC_RES;
    else
        return CgiResponse::CGI_RESPONSE_ERROR;
}

void CgiResponseParser::_insertResponse(CgiResponse& cgiResponse)
{
    std::vector<pair_t>::iterator it;

    for (it = _pairV.begin(); it != _pairV.end(); ++it)
        if (isCaseInsensitiveSame(it->first, "Status"))
            break;
    cgiResponse.setType(_determineType());
    switch (cgiResponse.getType()) {
    case CgiResponse::CLIENT_REDIR_RES:
        cgiResponse.setStatusCode(302);
        break;
    case CgiResponse::DOCUMENT_RES:
        if (it == _pairV.end()) {
            cgiResponse.setStatusCode(200);
            break;
        }
        /* fall-through */
    case CgiResponse::CLIENT_REDIRDOC_RES:
        cgiResponse.setStatusCode(stringToType<unsigned short>(it->second.substr(0, 3)));
        break;
    }
    if (it != _pairV.end())
        _pairV.erase(it);
    for (size_t i = 0; i < _pairV.size(); ++i)
        cgiResponse.addHeaderField(_pairV[i]);
    cgiResponse.setMessageBody(_messageBody);
}

void CgiResponseParser::parseCgiResponse(CgiResponse& cgiResponse, const std::string& raw)
{
    CgiResponseParser& parser = _getInstance();

    parser._init();
    try {
        parser._readLines(raw);
        parser._parseLines();
        parser._insertResponse(cgiResponse);
    }
    catch (unsigned short code) {
        cgiResponse.setType(CgiResponse::CGI_RESPONSE_ERROR);
    }
}
