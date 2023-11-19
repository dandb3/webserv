#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <vector>
# include <string>

struct server
{
	std::string server_name;
	short port;
	
};

class config
{
private:
	std::vector<struct server> _servers;

public:

};

#endif
