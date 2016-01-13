//
//  VROReticle.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/12/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface VROReticle : UIView

- (void)drawRect:(CGRect)rect;

@property (readwrite, nonatomic) float reticleSize;
@property (readwrite, nonatomic) float reticleThickness;

@end
