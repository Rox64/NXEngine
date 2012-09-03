
#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <cstdio>

#if defined(WIN32)
# define snprintf _snprintf
# define strcasecmp _stricmp
# define htole16(x) x

#endif

extern "C"
{
	void platform_sync_to_vblank(void);
};

FILE *fileopenRO(const char *fname);
FILE *fileopenRW(const char *fname, const char *mode);

#endif
