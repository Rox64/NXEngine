//
//  SDL_uikitviewcontroller+SDL_uikitviewcontroller_touch.m
//  CaveStory
//
//  Created by Pavlo Ilin on 18.11.12.
//
//

#import "SDL_uikitviewcontroller+SDL_uikitviewcontroller_touch.h"
#import <objc/runtime.h>
#import "touch_control_private.h"

static char tapOnceAssocKey;

SDL_uikitviewcontroller* viewcontroller = NULL;

void toggle_gesture_recognizer(int enabled)
{
    if (viewcontroller)
    {
        UITapGestureRecognizer* tapOnceRecogn = objc_getAssociatedObject(viewcontroller, &tapOnceAssocKey);
        
        if (enabled)
        {
            [viewcontroller.view addGestureRecognizer:tapOnceRecogn];
        }
        else
        {
            [viewcontroller.view removeGestureRecognizer:tapOnceRecogn];
        }
    }
}

@implementation SDL_uikitviewcontroller (SDL_uikitviewcontroller_touch)

- (void)viewDidLayoutSubviews
{
    UITapGestureRecognizer* tapOnceRecogn = [[UITapGestureRecognizer alloc] initWithTarget:self  action:@selector(tapOnce:)];
    
    tapOnceRecogn.numberOfTapsRequired = 1;
    
    objc_setAssociatedObject(self, &tapOnceAssocKey, tapOnceRecogn, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

    viewcontroller = self;
}

- (void)tapOnce:(UIGestureRecognizer *)gesture
{
    if (gesture.state == UIGestureRecognizerStateEnded)
    {
        CGPoint location = [gesture locationInView:gesture.view];
        
        location.x /= self.view.bounds.size.width;
        location.y /= self.view.bounds.size.height;
        
        NSLog(@"TAP GESTERE %f %f", location.x, location.y);
        
        tap(location.x, location.y);
    }
}

@end
