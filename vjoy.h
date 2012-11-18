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
void PreProcessInput();
void ProcessInput();

namespace ModeAware
{
    enum SpecScreens
    {
        ETextBox,
        ESaveLoad,
        EYesNo,
        EStageSelect1,
        EStageSelect2
    };
    
    
    bool wasTap(RectI rect);
    bool wasTap();
    
    void gameModeChanged(int newMode);
    void specScreenChanged(SpecScreens newScreen, bool enter);
}
    
} // namespace VJoy

#endif // VJOY_H__