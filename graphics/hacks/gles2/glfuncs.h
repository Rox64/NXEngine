#ifndef GLFUNCS_H__
#define GLFUNCS_H__

#ifdef __cplusplus
extern "C" 
{
#endif

struct GL_Functions
{
	#define SDL_PROC(ret,func,params) ret (APIENTRY *func) params;
	#include "glfuncs_list.h"
	#undef SDL_PROC
};


int LoadFunctions(struct GL_Functions * data);


#ifdef __cplusplus
} // extern "C" 
#endif

#endif