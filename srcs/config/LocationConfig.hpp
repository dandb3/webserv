#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <iostream>
#include <vector>
#include <map>

typedef std::map<std::string, std::vector<std::string> > t_directives;

class LocationConfig
{
private:
    /* data */
    t_directives _locationInfo;

public:
    LocationConfig();
    LocationConfig(const LocationConfig &ref);
    ~LocationConfig();

    LocationConfig &operator=(const LocationConfig &ref);

    // getter
    t_directives &getLocationInfo();
    std::vector<std::string> &getVariable(std::string &key);

    // setter
    void setLocationInfo(t_directives &locationInfo); // 필요한가? 보류
    void setVariable(std::string &key, std::vector<std::string> &value);
};

#endif
