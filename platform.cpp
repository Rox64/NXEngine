
#include <SDL.h>
#include <cstdio>
#include "config.h"
#include "platform.fdh"


char const* ro_filesys_path = "./";
char const* rw_filesys_path = "./";

#ifndef __SDLSHIM__

FILE *fileopen(char const* fname, char const* mode, char const* filesys_path)
{
   const size_t buf_size = 1024;
   static char buffer[buf_size];
   int res = snprintf(buffer, buf_size, "%s%s", filesys_path, fname);
   if (res < 0 || res >= buf_size)
      return NULL;

   return fopen(buffer, mode);
}

FILE *fileopenRO(const char *fname)
{
   return fileopen(fname, "rb", ro_filesys_path);
}

FILE *fileopenRW(const char *fname, const char *mode)
{
   
	return fileopen(fname, mode, rw_filesys_path);
}

#else

// FILE *fileopen(const char *fname, const char *mode)
// {
// 	return SDLS_fopen(fname, mode);
// }

#endif

