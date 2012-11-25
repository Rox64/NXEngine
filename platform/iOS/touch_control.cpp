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


extern "C"
{
    void tap(float x, float y)
    {
        if (sGO)
            sGO->tap(x, y);
    }
    
    void pan(float dx, float dy)
    {
        if (sGO)
            sGO->pan(dx, dy);
    }
    
    void pinch(float scale)
    {
        if (sGO)
            sGO->pinch(scale);
    }

}