#include <SDL_opengl.h>

#include "glfuncs.h"


int LoadFunctions(struct GL_Functions * data)
{
	#define SDL_PROC(ret,func,params) \
	    do { \
	        data->func = SDL_GL_GetProcAddress(#func); \
	        if ( ! data->func ) { \
	            SDL_SetError("Couldn't load GL function %s: %s\n", #func, SDL_GetError()); \
	            return -1; \
	        } \
	    } while ( 0 );

	#include "glfuncs_list.h"
	#undef SDL_PROC
    return 0;
}
