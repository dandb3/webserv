#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#define FAILURE -1
#define SUCCESS 0

#define CRLF "\r\n"
#define WHITESPACE " \t\n\r\f\v"

#include "Exception.hpp"

#ifndef _SYS_EVENT_H_
#define _SYS_EVENT_H_

#define EVFILT_READ (-1)
#define EVFILT_WRITE (-2)
#define EVFILT_AIO (-3)		 /* attached to aio requests */
#define EVFILT_VNODE (-4)	 /* attached to vnodes */
#define EVFILT_PROC (-5)	 /* attached to struct proc */
#define EVFILT_SIGNAL (-6)	 /* attached to struct proc */
#define EVFILT_TIMER (-7)	 /* timers */
#define EVFILT_MACHPORT (-8) /* Mach ports */
#define EVFILT_FS (-9)		 /* Filesystem events */

#define EVFILT_SYSCOUNT 9
#define EVFILT_THREADMARKER EVFILT_SYSCOUNT /* Internal use only */

struct kevent
{
	uintptr_t ident;	  /* identifier for this event */
	short filter;		  /* filter for event */
	unsigned short flags; /* general flags */
	unsigned int fflags;  /* filter-specific flags */
	intptr_t data;		  /* filter-specific data */
#ifdef KERNEL_PRIVATE
	user_addr_t udata; /* opaque user data identifier */
#else
	void *udata; /* opaque user data identifier */
#endif
};

#define EV_SET(kevp, a, b, c, d, e, f)    \
	do                                    \
	{                                     \
		struct kevent *__kevp__ = (kevp); \
		__kevp__->ident = (a);            \
		__kevp__->filter = (b);           \
		__kevp__->flags = (c);            \
		__kevp__->fflags = (d);           \
		__kevp__->data = (e);             \
		__kevp__->udata = (f);            \
	} while (0)

/* actions */
#define EV_ADD 0x0001	  /* add event to kq (implies enable) */
#define EV_DELETE 0x0002  /* delete event from kq */
#define EV_ENABLE 0x0004  /* enable event */
#define EV_DISABLE 0x0008 /* disable event (not reported) */

/* flags */
#define EV_ONESHOT 0x0010 /* only report one occurrence */
#define EV_CLEAR 0x0020	  /* clear event state after reporting */

#define EV_SYSFLAGS 0xF000 /* reserved by system */
#define EV_FLAG0 0x1000	   /* filter-specific flag */
#define EV_FLAG1 0x2000	   /* filter-specific flag */

/* returned values */
#define EV_EOF 0x8000	/* EOF detected */
#define EV_ERROR 0x4000 /* error, data contains errno */

#endif

#endif
