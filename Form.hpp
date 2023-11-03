#ifndef FORM_HPP
# define FORM_HPP

# include <map>
# include <vector>
# include <string>

class AForm
{
protected:
    std::map<std::string, std::vector<std::string> > _hdrField;
    std::string _msgBody;

    virtual void _insertStart(const std::string& start) = 0;
    void _insertHeader(const std::string& header);
    void _insertBody(const std::string& body);

public:
    void parse(const std::string& inMsg);

};

class RequestForm : public AForm
{
private:
    int _method;
    std::string _uri;
    std::string _protocol;

protected:
    virtual void _insertStart(const std::string& start);

};

class ResponseForm : public AForm
{
private:
    int _statCode;
    std::string _msg;
    std::string _protocol;

protected:
    virtual void _insertStart(const std::string& start);

};

#endif
