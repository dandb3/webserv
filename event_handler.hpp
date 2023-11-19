#ifndef EVENT_HANDLER_HPP
# define EVENT_HANDLER_HPP

# include <vector>
# include <set>
# include <unistd.h>
# include <sys/socket.h>
# include <sys/event.h>

# define EVENT_SIZE 10

class event_handler
{
public:
	enum
	{
		HTTP_REQUEST,
		HTTP_RESPONSE,
		CGI_REQUEST,
		CGI_RESPONSE,
		SERVER_LISTEN,
		SERVER_ERROR
	};

private:
	event_handler();
	event_handler(const event_handler& handler);
	event_handler& operator=(const event_handler& handler);

	std::set<int> _pipe_fds;
	std::set<int> _listen_fds;
	std::vector<struct kevent> _get_vec;	// const size equal to EVENT_SIZE
	std::vector<struct kevent> _set_vec;
	int _nevents;
	int _kq;

public:
	event_handler(int i);
	~event_handler();

	void event_listen();
	void event_catch();
	int event_type(int idx) const;			// throws an out_of_range exception when idx >= _nevents

	inline int get_nevents() const;

};

inline
int event_handler::get_nevents() const
{
	return this->_nevents;
}

#endif
