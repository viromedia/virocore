//
//  VROReticle.m
//  ViroRenderer
//
//  Created by Raj Advani on 1/12/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROReticle.h"

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

- (void)drawRect:(CGRect)rect {
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSetRGBStrokeColor(context, 0.33, 0.976, 0.968, 1.0);
    
    CGContextBeginPath(context);
    CGContextSetLineWidth(context, self.reticleThickness);
    
    CGContextAddArc(context,
                    rect.size.width  / 2.0 - self.reticleSize / 2.0,
                    rect.size.height / 2.0 - self.reticleSize / 2.0,
                    self.reticleSize,
                    0, M_PI * 2,
                    YES);
    
    CGContextClosePath(context);
    CGContextStrokePath(context);
}

@end
