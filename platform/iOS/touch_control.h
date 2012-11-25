//
//  touch_control.h
//  CaveStory
//
//  Created by Pavlo Ilin on 18.11.12.
//
//

#ifndef CaveStory_touch_control_h
#define CaveStory_touch_control_h

struct IGestureObserver
{
    virtual void tap(float x, float y) = 0;
    virtual void pan(float dx, float dy) = 0;
    virtual void pinch(float scale) = 0;
protected:
    virtual ~IGestureObserver() {}
};

void registerGetureObserver(IGestureObserver* observer);
void toggleGestureRecognizer(bool enabled);
void toggle_spec_gesture_recognizer(int enabled);

#endif
