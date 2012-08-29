
#ifndef _PLATFORM_H
#define _PLATFORM_H

extern "C"
{
	void platform_sync_to_vblank(void);
};

FILE *fileopenRO(const char *fname);
FILE *fileopenRW(const char *fname, const char *mode);

#endif
