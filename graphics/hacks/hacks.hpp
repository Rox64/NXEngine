#ifndef HACKS_HPP__
#define HACKS_HPP__


struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Rect;

namespace GraphicHacks {

bool Init(SDL_Renderer * renderer);



bool BatchBegin(SDL_Renderer * renderer, size_t max_count);
bool BatchAddCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                  const SDL_Rect * srcrect, const SDL_Rect * dstrect);
bool BatchEnd(SDL_Renderer * renderer);

} // namespace GraphicHacks



#endif // HACKS_HPP__