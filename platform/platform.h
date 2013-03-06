
#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <cstdio>

#if defined(WIN32)
# define snprintf _snprintf
# define strcasecmp _stricmp
#endif

#if defined (__APPLE__)
#  include "TargetConditionals.h"
#  if defined(TARGET_OS_IPHONE)
#     define IPHONE
#  endif
#endif

extern "C"
{
	void platform_sync_to_vblank(void);
};

bool setup_path(int argc, char** argv);

// Opens file from readonly place
FILE *fileopenRO(const char *fname);

// Opens file from place for permament storage
FILE *fileopenRW(const char *fname, const char *mode);

// Opens file from place for temporary storage.
// Its not guranteed, that file will exist on next application run.
FILE *fileopenCache(const char *fname, const char *mode);

#endif
