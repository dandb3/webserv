#ifndef CONVERTER_HPP
# define CONVERTER_HPP

# include <exception>
# include <string>
# define BUFSIZE 1024

class Converter
{
private:
	class EInput : std::exception
	{
	public:
		const char* what() const throw();

	};

	class EParse : std::exception
	{
	public:
		const char* what() const throw();

	};

private:
	static char _iBuf[BUFSIZE + 1];

	std::string _inMsg;
	std::string _outMsg;
	bool _eof;

	void _makeResponse();

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
