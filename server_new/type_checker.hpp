#ifndef TYPE_CHECKER_HPP
# define TYPE_CHECKER_HPP

# include <map>
# include <sys/event.h>

class type_checker
{
public:
    enum
    {
        SERV_LISTEN,
        SERV_HTTP_REQ,
        SERV_HTTP_RES,
        SERV_CGI_REQ,
        SERV_CGI_RES,
        SERV_ERROR,
    };

private:
    std::map<int, char> _type_m;

public:
    inline int get_type(const struct kevent& kev);
    inline void insert_type(int ident, int type);
    inline void erase_type(int ident);

};

inline int type_checker::get_type(const struct kevent& kev)
{
    if (kev.flags & EV_ERROR)
        return SERV_ERROR;
    else
        return _type_m[kev.ident];
}

inline void type_checker::insert_type(int ident, int type)
{
    _type_m.insert(std::make_pair(ident, type));
}

inline void type_checker::erase_type(int ident)
{
    _type_m.erase(ident);
}

#endif
