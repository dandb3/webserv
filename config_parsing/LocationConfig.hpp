#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <iostream>
#include <vector>
#include <map>

typedef std::map<std::string, std::vector<std::string>> t_directives;

class LocationConfig
{
private:
    /* data */
    std::string _path;
    // std::string _path_type; // location의 조정부호를 저장하는 변수
    t_directives _location_info;

public:
    LocationConfig();
    LocationConfig(const LocationConfig &ref);
    ~LocationConfig();

    LocationConfig &operator=(const LocationConfig &ref);

    // getter
    std::string &getPath();
    t_directives &getLocationInfo();
    std::vector<std::string> &getVariable(std::string &key);

    // setter
    void setPath(std::string &path);
    void setLocationInfo(t_directives &location_info); // 필요한가? 보류
    void setVariable(std::string &key, std::vector<std::string> &value);
};

#endif
