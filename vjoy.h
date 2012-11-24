#ifndef VJOY_H__
#define VJOY_H__

#include "nx_math.h"
#include "input.h"

union SDL_Event;

namespace VJoy
{

bool Init();
void Destroy();
void DrawAll();
void InjectInputEvent(SDL_Event const & event);
void PreProcessInput();
void ProcessInput();
    
    
    struct Preset
    {
        RectF positions[INPUT_COUNT];
        PointF pad_center;
        float pad_size;
    };
    
Preset const& getPreset(size_t num);
size_t getPresetsCount();
size_t getCurrentPresetNum();
Preset const& getCurrentSet();
void setCurrentSet(Preset const& preset);
void setFromPreset(size_t num);

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