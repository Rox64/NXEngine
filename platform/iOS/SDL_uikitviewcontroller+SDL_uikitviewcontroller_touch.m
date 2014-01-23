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
static char panAssocKey;
static char pinchAssocKey;

static SDL_uikitviewcontroller* viewcontroller = NULL;
static bool gesture_enabled = false;
static bool spec_gesture_enabled = false;

void toggle_gesture_recognizer(int enabled)
{
    if (gesture_enabled == enabled)
        return;
    
    if (viewcontroller)
    {
        UITapGestureRecognizer* tapOnceRecogn = objc_getAssociatedObject(viewcontroller, &tapOnceAssocKey);
        
        gesture_enabled = enabled;
        
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

void toggle_spec_gesture_recognizer(int enabled)
{
    if (spec_gesture_enabled == enabled)
        return;
    
    if (viewcontroller)
    {
        UIPanGestureRecognizer* panRecogn = objc_getAssociatedObject(viewcontroller, &panAssocKey);
        UIPanGestureRecognizer* pinchRecogn = objc_getAssociatedObject(viewcontroller, &pinchAssocKey);
        
        spec_gesture_enabled = enabled;
        
        if (enabled)
        {
            [viewcontroller.view addGestureRecognizer:panRecogn];
            [viewcontroller.view addGestureRecognizer:pinchRecogn];
        }
        else
        {
            [viewcontroller.view removeGestureRecognizer:panRecogn];
            [viewcontroller.view removeGestureRecognizer:pinchRecogn];
        }
    }
}

@implementation SDL_uikitviewcontroller (SDL_uikitviewcontroller_touch)

- (void)viewDidLayoutSubviews
{
    // tap once
    UITapGestureRecognizer* tapOnceRecogn = [[UITapGestureRecognizer alloc] initWithTarget:self  action:@selector(tapOnce:)];
    
    tapOnceRecogn.numberOfTapsRequired = 1;
    
    objc_setAssociatedObject(self, &tapOnceAssocKey, tapOnceRecogn, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

    // pan 
    UIPanGestureRecognizer* panRecogn = [[UIPanGestureRecognizer alloc] initWithTarget:self  action:@selector(handlePan:)];
    
    objc_setAssociatedObject(self, &panAssocKey, panRecogn, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    
    // pinch
    UIPinchGestureRecognizer* pinchRecogn = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(handlePinch:)];
    
    objc_setAssociatedObject(self, &pinchAssocKey, pinchRecogn, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

    
    viewcontroller = self;
}

- (void)tapOnce:(UIGestureRecognizer *)gesture
{
    if (gesture.state == UIGestureRecognizerStateEnded)
    {
        CGPoint location = [gesture locationInView:gesture.view];
        
        location.x /= self.view.bounds.size.width;
        location.y /= self.view.bounds.size.height;
        
        //NSLog(@"TAP GESTERE %f %f", location.x, location.y);
        
        tap(location.x, location.y);
    }
}

- (void)handlePan:(UIPanGestureRecognizer *)gesture
{
    CGPoint p = [gesture locationInView:gesture.view];
    
    CGPoint t = [gesture translationInView:gesture.view];
    [gesture setTranslation:CGPointZero inView:gesture.view];
    
    t.x /= gesture.view.bounds.size.width;
    t.y /= gesture.view.bounds.size.height;
    
    p.x /= gesture.view.bounds.size.width;
    p.y /= gesture.view.bounds.size.height;
    
    
    //NSLog(@"PAN GESTERE %f %f", t.x, t.y);
    
    pan(p.x, p.y, t.x, t.y);
}

- (void)handlePinch:(UIPinchGestureRecognizer *)gesture
{
    //NSLog(@"Pinch scale: %f", gesture.scale);
    
    pinch(gesture.scale, gesture.state == UIGestureRecognizerStateEnded);
}

@end
