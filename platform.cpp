
#include <SDL.h>
#include <cstdio>
#include "config.h"
#include "platform.h"
#include "common/basics.h"

char const* ro_filesys_path = "./";
char const* rw_filesys_path = "./";
char const* ca_filesys_path = "./";

FILE *fileopen(char const* fname, char const* mode, char const* filesys_path)
{
   //stat("fileopen %s %s %s", fname, mode, filesys_path);

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

FILE *fileopenCache(const char *fname, const char *mode)
{
    
	return fileopen(fname, mode, ca_filesys_path);
}

void setup_path()
{
#ifdef IPHONE
    ro_filesys_path = "./game_resources/";
    rw_filesys_path = "../Documents/";
    ca_filesys_path = "../Library/Caches/";
#elif USE_RO_FILESYS
    ro_filesys_path = "./ro/";
    rw_filesys_path = "./rw/";
    ca_filesys_path = "./rw/";
#endif
}

