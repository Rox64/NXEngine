#ifndef VJOY_H__
#define VJOY_H__


union SDL_Event;

namespace VJoy
{

bool Init();
void Destroy();
void DrawAll();
void InjectInputEvent(SDL_Event const & event);
void ProcessInput();

} // namespace VJoy

#endif // VJOY_H__