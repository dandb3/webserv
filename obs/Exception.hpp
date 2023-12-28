#ifndef EXCEPTION_HPP
# define EXCEPTION_HPP

# include <exception>

class err_syscall : std::exception
{
public:
	const char* what() const throw();

};

class err_input : std::exception
{
public:
    const char* what() const throw();

};

class err_parse : std::exception
{
public:
    const char* what() const throw();

};

#endif
