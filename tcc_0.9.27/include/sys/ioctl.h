// sys/ioctl.h
// Libunistd Copyright 2016 Robin.Rowe@CinePaint.org
// License open source MIT

#ifndef sys_ioctl_h
#define sys_ioctl_h

//#include "../../portable/stub.h"

#ifdef __cplusplus
extern "C" {
#else
#define inline __inline
#endif

inline
int ioctl(int fd, unsigned long request, ...)
{   //	STUB_NEG(ioctl);
}

struct winsize
{
	unsigned short ws_row;		/* rows, in characters */
	unsigned short ws_col;		/* columns, in characters */
	unsigned short ws_xpixel;	/* horizontal size, pixels */
	unsigned short ws_ypixel;	/* vertical size, pixels */
};

// include net/if.h if you need struct ifreq

#ifdef __cplusplus
}
#endif

#endif
