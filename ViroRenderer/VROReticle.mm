//
//  VROReticle.m
//  ViroRenderer
//
//  Created by Raj Advani on 1/12/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROReticle.h"
#import <QuartzCore/QuartzCore.h>
#import "VROScreenUIView.h"
#import "VROMath.h"

static const float kTriggerAnimationDuration = 0.4;
static const float kTriggerAnimationInnerCircleThicknessMultiple = 3;
static const float kTriggerAnimationWhiteCircleMultiple = 4;

@interface VROReticle ()

@property (readwrite, nonatomic) float t;
@property (readwrite, nonatomic) float animationStartSeconds;
@property (readwrite, nonatomic) float endThickness;
@property (readwrite, nonatomic) BOOL  animationActive;

@end

@implementation VROReticle

- (id)init {
    self = [super initWithFrame:CGRectZero];
    if (self) {
        self.backgroundColor = [UIColor clearColor];
        
        // Sensible defaults
        self.reticleSize = 2.5;
        self.reticleThickness = 1;
    }
    
    return self;
}

- (void)trigger {
    self.animationStartSeconds = CACurrentMediaTime();
    self.endThickness = self.reticleThickness * kTriggerAnimationInnerCircleThicknessMultiple;
    self.animationActive = YES;
    
    [(VROScreenUIView *)self.superview setNeedsUpdate];
}

- (void)renderRect:(CGRect)rect context:(CGContextRef)context {
    float thickness = self.reticleThickness;
        
    if (self.animationActive) {
        float t = (CACurrentMediaTime() - self.animationStartSeconds) / kTriggerAnimationDuration;
        
        float whiteAlpha = 0.0;
        if (t < 0.5) {
            thickness = VROMathInterpolate(t, 0, 0.5, self.reticleThickness, self.endThickness);
            whiteAlpha = VROMathInterpolate(t, 0, 0.5, 0, 1.0);
        }
        else {
            thickness = VROMathInterpolate(t, 0.5, 1.0, self.endThickness, self.reticleThickness);
            whiteAlpha = VROMathInterpolate(t, 0.5, 1.0, 1.0, 0);
        }
        float whiteRadius = VROMathInterpolate(t, 0, 1.0, self.reticleSize, self.reticleSize * kTriggerAnimationWhiteCircleMultiple);
        
        CGContextSetRGBFillColor(context, 1.0, 1.0, 1.0, whiteAlpha);
        
        CGContextBeginPath(context);
        CGContextSetLineWidth(context, thickness);
        
        CGContextAddArc(context,
                        rect.size.width  / 2.0,
                        rect.size.height / 2.0,
                        whiteRadius,
                        0, M_PI * 2,
                        YES);
        
        CGContextClosePath(context);
        CGContextFillPath(context);
        
        if (t < 1.0) {
            [(VROScreenUIView *)self.superview setNeedsUpdate];
        }
        else {
            self.animationActive = NO;
        }
    }

    CGContextSetRGBStrokeColor(context, 0.33, 0.976, 0.968, 1.0);
    
    CGContextBeginPath(context);
    CGContextSetLineWidth(context, thickness);
    
    CGContextAddArc(context,
                    rect.size.width  / 2.0,
                    rect.size.height / 2.0,
                    self.reticleSize,
                    0, M_PI * 2,
                    YES);
    
    CGContextClosePath(context);
    CGContextStrokePath(context);
}

@end
