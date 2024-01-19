#ifndef CGI_RESPONSE_PARSER_HPP
#define CGI_RESPONSE_PARSER_HPP

#include <vector>
#include "parse.hpp"
#include "../modules/CgiResponseModule.hpp"

/**
 * singleton으로 구성.
 * parseCgiResponse를 통해서 내부적으로 이 singleton에 접근할 수 있다.
*/
class CgiResponseParser
{
private:
    enum
    {
        HDR_CONTENT_TYPE = 0,
        HDR_LOCAL_LOCATION,
        HDR_CLIENT_LOCATION,
        HDR_STATUS,
        HDR_PROTOCOL,
        HDR_EXTENSION
    };

    static CgiResponseParser _cgiResponseParser;
    static CgiResponseParser& _getInstance();

    std::vector<std::string> _lineV;
    std::vector<pair_t> _pairV;
    std::string _messageBody;
    std::vector<int> _fieldCnt;

    CgiResponseParser();
    CgiResponseParser(const CgiResponseParser& cgiResponseParser);
    CgiResponseParser& operator=(const CgiResponseParser& cgiResponseParser);
    ~CgiResponseParser();

    void _init();
    void _readLines(const std::string& raw);
    void _parseLines();
    char _determineType();
    void _insertResponse(CgiResponse& cgiResponse);

public:
    static void parseCgiResponse(CgiResponse& cgiResponse, const std::string& raw);

};

#endif
