#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <string>
# include <map>

class Response
{
private:
    int _statcode;
    std::string _msg;
    std::string _protocol;
    std::map<std::string, std::vector<std::string> > _hdrField;
    std::string _msgBody;

};

#endif
