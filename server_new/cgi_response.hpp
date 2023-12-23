#ifndef CGI_RESPONSE_HPP
# define CGI_RESPONSE_HPP

# include <string>

class cgi_response
{
private:
	std::string _response_msg;

public:
	void read_input();

	inline const std::string& get_response_msg() const;

};

inline const std::string& cgi_response::get_response_msg() const
{
	return _response_msg;
}

#endif
