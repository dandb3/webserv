#ifndef CONVERTER_HPP
# define CONVERTER_HPP

# include <string>
# include "Exception.hpp"
# define BUFSIZE 1024

class Converter
{
private:
	static char _input_buf[BUFSIZE + 1];

	std::string _in_msg;
	std::string _out_msg;
	bool _eof;

	void _convert();

public:
	Converter();
	Converter(const Converter& conv);
	Converter& operator=(const Converter& conv);
	~Converter();

	bool eof() const;
	void add(int readfd);
	void response(int writefd) const;

};

#endif
