//
//  SDL_uikitview+touch.m
//  CaveStory
//
//  Created by Pavlo Ilin on 18.11.12.
//
//

#import "SDL_uikitview+touch.h"

@implementation SDL_uikitview (touch)

- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gestureRecognizer
{
    return YES;
}



@end
