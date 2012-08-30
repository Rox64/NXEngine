#ifndef VJOY_H__
#define VJOY_H__


union SDL_Event;

namespace VJoy
{

bool Init();
void Destroy();
void DrawAll();
void ProcessInput(SDL_Event const & event);

} // namespace VJoy

#endif // VJOY_H__