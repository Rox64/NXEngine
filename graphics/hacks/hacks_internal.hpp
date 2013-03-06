#ifndef HACKS_INTERNAL_HPP__
#define HACKS_INTERNAL_HPP__ 

#include <SDL.h>
extern "C" {
#include <SDL_sysrender.h>
}

namespace GraphicHacks {

struct Hack
{
	virtual char const* name() const = 0;

	virtual bool Init(SDL_Renderer * renderer) = 0;

	virtual bool BatchBegin(SDL_Renderer * renderer, size_t max_count) = 0;
	virtual bool BatchAddCopy(SDL_Renderer * renderer, SDL_Texture * texture,
	                          const SDL_Rect * srcrect, const SDL_FRect * dstrect) = 0;
	virtual bool BatchEnd(SDL_Renderer * renderer) = 0;

protected:
	virtual ~Hack() {}
};

}



#endif