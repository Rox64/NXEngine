//
//  RectI.h
//  CaveStory
//
//  Created by Pavlo Ilin on 18.11.12.
//
//

#ifndef CaveStory_RectI_h
#define CaveStory_RectI_h

struct RectI
{
    int x, y;
    int w, h;
    
    RectI() : x(0), y(0), w(0), h(0) {}
    RectI(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
    
};

#endif
