//
//  touch_control.h
//  CaveStory
//
//  Created by Pavlo Ilin on 18.11.12.
//
//

#ifndef CaveStory_touch_control_h
#define CaveStory_touch_control_h

class IGestureObserver;

void registerGetureObserver(IGestureObserver* observer);
void toggleGestureRecognizer(bool enabled);
void toggleSpecGestureRecognizer(bool enabled);

#endif
