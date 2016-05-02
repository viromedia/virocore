//
//  VROScreenUIView.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/12/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "VRORenderContext.h"
#import "VRODriver.h"
#import "VROLayer.h"
#import "VROReticle.h"
#import <memory>

class VROEye;

/*
 UIView for rendering a HUD in screen space.
 */
@interface VROScreenUIView : UIView

- (instancetype)init;

- (void)updateWithContext:(const VRODriver *)context;
- (void)renderEye:(VROEyeType)eye
withRenderContext:(const VRORenderContext *)renderContext
    driver:(const VRODriver *)driver;

- (void)setReticleEnabled:(BOOL)enabled;
- (void)setNeedsUpdate;

- (void)setDepth:(float)depth;

@property (readonly, nonatomic) VROReticle *reticle;

@end
