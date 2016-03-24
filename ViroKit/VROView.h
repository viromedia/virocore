//
//  VROView.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import "VRORenderDelegate.h"
#import "VROScreenUIView.h"
#import <memory>

class VROScene;
enum class VROTimingFunctionType;

@interface VROView : MTKView <MTKViewDelegate>

@property (nonatomic, unsafe_unretained) IBOutlet id <VRORenderDelegate> renderDelegate;
@property (nonatomic) BOOL vrModeEnabled;
@property (nonatomic) BOOL vignetteEnabled;
@property (nonatomic) BOOL chromaticAberrationCorrectionEnabled;

@property (nonatomic, readonly) VROScreenUIView *HUD;
@property (readwrite, nonatomic) std::shared_ptr<VROScene> scene;

- (instancetype)initWithFrame:(CGRect)frame;
- (VRORenderContext *)renderContext;

- (void)setScene:(std::shared_ptr<VROScene>)scene animated:(BOOL)animated;
- (void)setScene:(std::shared_ptr<VROScene>)scene duration:(float)seconds
  timingFunction:(VROTimingFunctionType)timingFunctionType
           start:(std::function<void(VROScene *const incoming, VROScene *const outgoing)>)start
        animator:(std::function<void(VROScene *const incoming, VROScene *const outgoing, float t)>)animator
             end:(std::function<void(VROScene *const incoming, VROScene *const outgoing)>)end;

- (void)setPosition:(VROVector3f)position;
- (void)setBaseRotation:(VROQuaternion)rotation;
- (float)worldPerScreenAtDepth:(float)distance;

@end
