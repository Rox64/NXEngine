#ifndef VJOY_H__
#define VJOY_H__

#include "RectI.h"

union SDL_Event;

namespace VJoy
{

bool Init();
void Destroy();
void DrawAll();
void InjectInputEvent(SDL_Event const & event);
void ProcessInput();

namespace ModeAware
{
    bool isPressedInCurrentMode(RectI rect);
}
    
} // namespace VJoy

#endif // VJOY_H__