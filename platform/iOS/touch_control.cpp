//
//  touch_control.cpp
//  CaveStory
//
//  Created by Pavlo Ilin on 18.11.12.
//
//

#include "touch_control.h"
#include "touch_control_private.h"

static IGestureObserver* sGO = 0;

void registerGetureObserver(IGestureObserver* observer)
{
    sGO = observer;
}

void toggleGestureRecognizer(bool enabled)
{
    toggle_gesture_recognizer(enabled);
}

void toggleSpecGestureRecognizer(bool enabled)
{
    toggle_spec_gesture_recognizer(enabled);
}

extern "C"
{
    void tap(float x, float y)
    {
        if (sGO)
            sGO->tap(x, y);
    }
    
    void pan(float x, float y, float dx, float dy)
    {
        if (sGO)
            sGO->pan(x, y, dx, dy);
    }
    
    void pinch(float scale, bool is_end)
    {
        if (sGO)
            sGO->pinch(scale, is_end);
    }

}