//
//  VROSceneController.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/25/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <memory>
#import "VROVector3f.h"
#import "VRORenderDelegate.h"

class VROScene;
class VRORenderContext;
class VROHoverDelegate;

@interface VROSceneController : NSObject

@property (readwrite, nonatomic) std::shared_ptr<VROHoverDelegate> hoverDelegate;

- (id)init;

- (void)sceneWillAppear:(VRORenderContext *)context;
- (void)sceneDidAppear:(VRORenderContext *)context;
- (void)sceneWillDisappear:(VRORenderContext *)context;
- (void)sceneDidDisappear:(VRORenderContext *)context;

- (void)startIncomingTransition:(VRORenderContext *)context duration:(float)duration;
- (void)startOutgoingTransition:(VRORenderContext *)context duration:(float)duration;
- (void)endIncomingTransition:(VRORenderContext *)context;
- (void)endOutgoingTransition:(VRORenderContext *)context;
- (void)animateIncomingTransition:(VRORenderContext *)context percentComplete:(float)t;
- (void)animateOutgoingTransition:(VRORenderContext *)context percentComplete:(float)t;

- (void)sceneWillRender:(const VRORenderContext *)context;
- (void)reticleTapped:(VROVector3f)ray context:(const VRORenderContext *)context;
- (std::shared_ptr<VROScene>)scene;

@end
