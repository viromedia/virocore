//
//  VROView.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "VRORenderDelegate.h"
#import "VROQuaternion.h"
#import "VROCamera.h"
#import <memory>

class VROSceneController;
class VROScene;
class VROReticle;
class VRONode;
class VRORenderer;
class VROChoreographer;
class VROFrameSynchronizer;
enum class VROTimingFunctionType;

typedef void (^VROViewValidApiKeyBlock)(BOOL);

@protocol VROView <NSObject>

@required

@property (nonatomic, weak) IBOutlet id <VRORenderDelegate> renderDelegate;
@property (readonly, nonatomic) std::shared_ptr<VRORenderer> renderer;
@property (readonly, nonatomic) std::shared_ptr<VROChoreographer> choreographer;
@property (readwrite, nonatomic) std::shared_ptr<VROSceneController> sceneController;

- (void)setSceneController:(std::shared_ptr<VROSceneController>)sceneController;
- (void)setSceneController:(std::shared_ptr<VROSceneController>)sceneController duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType;

- (void)setPointOfView:(std::shared_ptr<VRONode>)node;
- (void)setDebugHUDEnabled:(BOOL)enabled;
- (void)recenterTracking;

- (BOOL)setShadowsEnabled:(BOOL)enabled;
- (BOOL)setHDREnabled:(BOOL)enabled;
- (BOOL)setPBREnabled:(BOOL)enabled;
- (BOOL)setBloomEnabled:(BOOL)enabled;

- (std::shared_ptr<VROFrameSynchronizer>)frameSynchronizer;

@end
