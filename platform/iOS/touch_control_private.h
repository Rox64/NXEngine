//
//  touch_control_private.h
//  CaveStory
//
//  Created by Pavlo Ilin on 18.11.12.
//
//

#ifndef CaveStory_touch_control_private_h
#define CaveStory_touch_control_private_h

#ifdef __cplusplus
extern "C"
{
#endif
    void tap(float x, float y);
    
    void pinch(float scale);
    
    void pan(float dx, float dy);
    
    void toggle_gesture_recognizer(int enabled);
    
    void toggle_spec_gesture_recognizer(int enabled);

#ifdef __cplusplus
}
#endif

#endif
