//
//  VROViewMetal.m
//  ViroRenderer
//
//  Created by Raj Advani on 4/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROViewMetal.h"
#import "VRODriverMetal.h"
#import "VROAllocationTracker.h"

@interface VROViewMetal ()

@property (readwrite, nonatomic) std::weak_ptr<VRODriverMetal> driver;

@end

@implementation VROViewMetal

- (id)initWithFrame:(CGRect)frame device:(id <MTLDevice>)device
             driver:(std::shared_ptr<VRODriverMetal>)driver {
    self = [super initWithFrame:frame];
    if (self) {
        self.device = device;
        self.driver = driver;
        self.delegate = self;
        self.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    }
    
    return self;
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    std::shared_ptr<VRODriverMetal> driver = self.driver.lock();
    if (driver) {
        driver->driveFrame();
    }
    
    ALLOCATION_TRACKER_PRINT();
}

@end
