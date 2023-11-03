#ifndef EXCEPTION_HPP
# define EXCEPTION_HPP

# include <exception>

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

#endif
