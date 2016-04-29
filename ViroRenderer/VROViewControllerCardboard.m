//
//  VROViewControllerCardboard.m
//  ViroRenderer
//
//  Created by Raj Advani on 4/28/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROViewControllerCardboard.h"
#import "VROCardboardRenderLoop.h"
#import "VROViewCardboard.h"

@interface VROViewControllerCardboard () {

    VROViewCardboard *_cardboardView;
    VROCardboardRenderLoop *_renderLoop;
    
}

@end

@implementation VROViewControllerCardboard

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    
    VROViewCardboard *cardboardView = (VROViewCardboard *) self.view;
    cardboardView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    cardboardView.vrModeEnabled = YES;
    
    _renderLoop = [[VROCardboardRenderLoop alloc] initWithRenderTarget:self.view
                                                              selector:@selector(render)];
}

- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
    
    // Invalidate the render loop so that it removes the strong reference to cardboardView.
    [_renderLoop invalidate];
    _renderLoop = nil;
}

@end
