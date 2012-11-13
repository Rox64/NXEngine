
#include <SDL.h>
#include <cerrno>
#include <cstdio>
#include <sys/stat.h>

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

bool is_dir(char const* path)
{
    struct stat sb;
    if (stat(path, &sb))
    {
        int saved_errno = errno;
        char* err = strerror(saved_errno);
        return false;
    }
    bool res = S_ISDIR(sb.st_mode);
    return res;
}

bool check_mkdir(const char* path, mode_t mode = 0755)
{
    if (!is_dir(path))
    {
        if (mkdir(path, mode))
        {
            int saved_errno = errno;
            char* err = strerror(saved_errno);
            return false;
        }
    }
    
    return true;
}

bool setup_path(int argc, char** argv)
{
/*
    const size_t buf_size = 512;
    char buffer[buf_size];
    
    if (!getcwd(buffer, buf_size))
    {
        exit(1);
    }
*/
    
    
    
#ifdef IPHONE
    bool sandboxed = false;
    char* r = strstr(argv[0], "/var/mobile/Applications/");
    if (r)
    {
        sandboxed = true;
    }
    
    if (sandboxed)
    {
        rw_filesys_path = "../Documents/";
        ca_filesys_path = "../Library/Caches/";
    }
    else
    {
#define DOCS   "/var/mobile/Library/"
#define CACHES "/var/mobile/Library/Caches/"
#define SUFFIX "CaveStory/"
        
//        if (!is_dir(DOCS) || !is_dir(CACHES))
//            return false;
        
        rw_filesys_path = DOCS   SUFFIX;
        ca_filesys_path = CACHES SUFFIX;
        
        if (!check_mkdir(rw_filesys_path) || ! check_mkdir(ca_filesys_path))
            return false;
#undef SUFFIX
#undef CACHES
#undef DOCS
        
    }
    
    ro_filesys_path = "./game_resources/";
    
#elif USE_RO_FILESYS
    ro_filesys_path = "./ro/";
    rw_filesys_path = "./rw/";
    ca_filesys_path = "./rw/";
#endif
    
    return true;
}

