#include "../../config.h"
#include "../../common/basics.h"

#include "hacks.hpp"
#include "hacks_internal.hpp"

using namespace GraphicHacks;


//#define HACK_OPENGL
//#define HACK_GLES


#if defined(HACK_OPENGL)
extern Hack* hack_gl; 
#endif

#if defined(HACK_GLES)
extern Hack* hack_gles2; 
#endif

static Hack * const hacks[] = 
{
#if defined(HACK_OPENGL)
	hack_gl,
#endif
#if defined(HACK_GLES)
	hack_gles2,
#endif
	NULL
};

static Hack* working_hack = NULL;


///


bool GraphicHacks::Init(SDL_Renderer * renderer)
{
	SDL_RendererInfo info;
	if (SDL_GetRendererInfo(renderer, &info))
	{
		staterr("Unable to get info about renderer");
		return true;
	}

	for (Hack * const * pph = &hacks[0]; *pph; ++pph)
	{
		Hack& h = **pph;

		if (0 == strcmp(info.name, h.name()))
		{
			stat("found graphic hack %s", h.name());

			if (h.Init(renderer))
				return true;

			working_hack = *pph;
			break;
		}
	}


	if (!working_hack)
	{
		staterr("no graphic hacks found");
		return true;
	}

	return false;
}


bool GraphicHacks::BatchBegin(SDL_Renderer * renderer, size_t max_count)
{
	if (working_hack)
		return working_hack->BatchBegin(renderer, max_count);
	else
		return true;
}

bool GraphicHacks::BatchAddCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                  const SDL_Rect * srcrect, const SDL_Rect * dstrect)
{
	if (!working_hack)
		return true;




	return working_hack->BatchAddCopy(renderer, texture, &real_srcrect, &frect);

}

bool GraphicHacks::BatchEnd(SDL_Renderer * renderer)
{
	if (working_hack)
		return working_hack->BatchEnd(renderer);
	else
		return true;
}
