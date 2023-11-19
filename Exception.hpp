#ifndef EXCEPTION_HPP
# define EXCEPTION_HPP

# include <exception>

class Err_syscall : std::exception
{
public:
	const char* what() const throw();

};

class Err_input : std::exception
{
public:
    const char* what() const throw();

};

class Err_parse : std::exception
{
public:
    const char* what() const throw();

};

#endif
