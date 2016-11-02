//
//  VRORenderLoopCardboard.m
//  ViroRenderer
//
//  Created by Raj Advani on 4/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VRORenderLoopCardboard.h"
#import "VROViewCardboard.h"

@implementation VRORenderLoopCardboard {
  NSThread *_renderThread;
  CADisplayLink *_displayLink;
  BOOL _paused;
}

- (instancetype)initWithRenderTarget:(id)target selector:(SEL)selector {
  if (self = [super init]) {
    _displayLink = [CADisplayLink displayLinkWithTarget:target selector:selector];
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillResignActive:)
                                                 name:UIApplicationWillResignActiveNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidBecomeActive:)
                                                 name:UIApplicationDidBecomeActiveNotification
                                               object:nil];
  }
  return self;
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Public

- (void)invalidate {
    [_displayLink invalidate];
    _displayLink = nil;
}

- (BOOL)paused {
    return _paused;
}

- (void)setPaused:(BOOL)paused {
    _paused = paused;
    _displayLink.paused = paused;
}

#pragma mark - NSNotificationCenter

- (void)applicationWillResignActive:(NSNotification *)notification {
    [self threadPause];
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    _displayLink.paused = _paused;
}

#pragma mark - Background thread rendering.

- (void)threadMain {
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    CFRunLoopRun();
}

- (void)threadPause {
  _displayLink.paused = YES;
}

- (void)renderThreadStop {
    [_displayLink invalidate];
    _displayLink = nil;
    
    CFRunLoopStop(CFRunLoopGetCurrent());
    
    // Ensure we release the last reference to self on the main thread. The ivar: _renderThread in the
    // dispatch block implicitly retains a strong reference to self.
    // See 'The deallocation problem' in:
    // https://developer.apple.com/library/ios/technotes/tn2109/_index.html
    dispatch_async(dispatch_get_main_queue(), ^{
        [_renderThread cancel];
        _renderThread = nil;
    });
}

@end

@implementation VRORenderLoopTarget

- (void)render {
    if (self.cardboardView && self.cardboardView.superview) {
        [self.cardboardView render];
    }
}

@end
