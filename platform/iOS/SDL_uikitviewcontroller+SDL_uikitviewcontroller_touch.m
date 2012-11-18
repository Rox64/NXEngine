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
static char tapTwiceAssocKey;

SDL_uikitviewcontroller* viewcontroller = NULL;

void toggle_gesture_recognizer(int enabled)
{
    if (viewcontroller)
    {
        UITapGestureRecognizer* tapOnceRecogn = objc_getAssociatedObject(viewcontroller, &tapOnceAssocKey);
        UITapGestureRecognizer* tapTwiceRecogn = objc_getAssociatedObject(viewcontroller, &tapTwiceAssocKey);
        
        if (enabled)
        {
            [viewcontroller.view addGestureRecognizer:tapOnceRecogn];
            [viewcontroller.view addGestureRecognizer:tapTwiceRecogn];
        }
        else
        {
            [viewcontroller.view removeGestureRecognizer:tapOnceRecogn];
            [viewcontroller.view removeGestureRecognizer:tapTwiceRecogn];
        }
    }
}

@implementation SDL_uikitviewcontroller (SDL_uikitviewcontroller_touch)

-(void) viewLoad
{
    //[super viewLoad];
}

- (void)viewDidLayoutSubviews
{
    UITapGestureRecognizer* tapOnceRecogn = [[UITapGestureRecognizer alloc] initWithTarget:self  action:@selector(tapOnce:)];
    UITapGestureRecognizer* tapTwiceRecogn = [[UITapGestureRecognizer alloc] initWithTarget:self  action:@selector(tapTwice:)];
    
    tapOnceRecogn.numberOfTapsRequired = 1;
    tapTwiceRecogn.numberOfTapsRequired = 2;
    
    [tapOnceRecogn requireGestureRecognizerToFail:tapTwiceRecogn];
    
    //[self.view addGestureRecognizer:tapOnce];
    //[self.view addGestureRecognizer:tapTwice];
    
    objc_setAssociatedObject(self, &tapOnceAssocKey, tapOnceRecogn, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    objc_setAssociatedObject(self, &tapTwiceAssocKey, tapTwiceRecogn, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

    viewcontroller = self;
    
    //toggle_gesture_recognizer(1);
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
- (void)tapTwice:(UIGestureRecognizer *)gesture
{
    if (gesture.state == UIGestureRecognizerStateEnded)
    {
        CGPoint location = [gesture locationInView:gesture.view];
        
        location.x /= self.view.bounds.size.width;
        location.y /= self.view.bounds.size.height;
        
        NSLog(@"DOUBLE TAP GESTURE %f %f", location.x, location.y);
        
        double_tap(location.x, location.y);
    }
}

@end
