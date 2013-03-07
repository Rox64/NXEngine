#include "../../config.h"
#include "../../common/basics.h"

#include <cstdlib>

#include "hacks.hpp"
#include "hacks_internal.hpp"

using namespace GraphicHacks;


//#define HACK_OPENGL
#define HACK_GLES


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

    SDL_Rect real_srcrect = { 0, 0, 0, 0 };
    SDL_Rect real_dstrect = { 0, 0, 0, 0 };
    SDL_FRect frect;

    if (renderer != texture->renderer) {
        SDL_SetError("Texture was not created with this renderer");
        return true;
    }

    real_srcrect.x = 0;
    real_srcrect.y = 0;
    real_srcrect.w = texture->w;
    real_srcrect.h = texture->h;
    if (srcrect) {
        if (!SDL_IntersectRect(srcrect, &real_srcrect, &real_srcrect)) {
            return 0;
        }
    }

    SDL_RenderGetViewport(renderer, &real_dstrect);
    real_dstrect.x = 0;
    real_dstrect.y = 0;
    if (dstrect) {
        if (!SDL_IntersectRect(dstrect, &real_dstrect, &real_dstrect)) {
            return 0;
        }
        /* Clip srcrect by the same amount as dstrect was clipped */
        if (dstrect->w != real_dstrect.w) {
            int deltax = (real_dstrect.x - dstrect->x);
            int deltaw = (real_dstrect.w - dstrect->w);
            real_srcrect.x += (deltax * real_srcrect.w) / dstrect->w;
            real_srcrect.w += (deltaw * real_srcrect.w) / dstrect->w;
        }
        if (dstrect->h != real_dstrect.h) {
            int deltay = (real_dstrect.y - dstrect->y);
            int deltah = (real_dstrect.h - dstrect->h);
            real_srcrect.y += (deltay * real_srcrect.h) / dstrect->h;
            real_srcrect.h += (deltah * real_srcrect.h) / dstrect->h;
        }
    }

    if (texture->native) {
        texture = texture->native;
    }

    /* Don't draw while we're hidden */
    if (renderer->hidden) {
        return 0;
    }

    frect.x = real_dstrect.x * renderer->scale.x;
    frect.y = real_dstrect.y * renderer->scale.y;
    frect.w = real_dstrect.w * renderer->scale.x;
    frect.h = real_dstrect.h * renderer->scale.y;


	return working_hack->BatchAddCopy(renderer, texture, &real_srcrect, &frect);

}

bool GraphicHacks::BatchEnd(SDL_Renderer * renderer)
{
	if (working_hack)
		return working_hack->BatchEnd(renderer);
	else
		return true;
}
