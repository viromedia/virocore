//
//  VROSceneController.m
//  ViroRenderer
//
//  Created by Raj Advani on 3/25/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROSceneController.h"
#import "VROSceneControlleriOS.h"
#import "VROHoverDelegate.h"
#import "VROTransaction.h"
#import "VROScene.h"
#import "VROMaterial.h"
#import "VROGeometry.h"
#import "VRONode.h"
#import "VROReticleSizeListener.h"
#import "VROFrameSynchronizer.h"
#import "VROView.h"
#import <map>

@interface VROSceneController ()

@property (readwrite, nonatomic) std::shared_ptr<VROHoverDelegate> hoverDelegate;
@property (readwrite, nonatomic) std::shared_ptr<VROSceneControlleriOS> internal;

@end

@implementation VROSceneController

- (id)init {
    self = [super init];
    if (self) {
        self.internal = std::make_shared<VROSceneControlleriOS>(self);
    }
    
    return self;
}

- (std::shared_ptr<VROScene>)scene {
    return self.internal->getScene();
}

- (void)setHoverEnabled:(BOOL)enabled boundsOnly:(BOOL)boundsOnly {
    self.internal->setHoverEnabled(enabled, boundsOnly);
}

#pragma mark - Delegate Methods

- (void)sceneWillAppear:(VRORenderContext *)context driver:(VRODriver *)driver {

}

- (void)sceneDidAppear:(VRORenderContext *)context driver:(VRODriver *)driver {
    
}

- (void)sceneWillDisappear:(VRORenderContext *)context driver:(VRODriver *)driver {
    
}

- (void)sceneDidDisappear:(VRORenderContext *)context driver:(VRODriver *)driver {
    
}

- (void)startIncomingTransition:(VRORenderContext *)context duration:(float)seconds {    

}

- (void)startOutgoingTransition:(VRORenderContext *)context duration:(float)seconds {

}

- (void)endIncomingTransition:(VRORenderContext *)context {
    
}

- (void)endOutgoingTransition:(VRORenderContext *)context {
    
}

- (void)animateIncomingTransition:(VRORenderContext *)context percentComplete:(float)t {
    
}

- (void)animateOutgoingTransition:(VRORenderContext *)context percentComplete:(float)t {
    
}

- (void)sceneWillRender:(const VRORenderContext *)context {
    
}

- (BOOL)isHoverable:(std::shared_ptr<VRONode>)node {
    return NO;
}

- (void)hoverOnNode:(std::shared_ptr<VRONode>)node {
    
}

- (void)hoverOffNode:(std::shared_ptr<VRONode>)node {
    
}

- (void)reticleTapped:(VROVector3f)ray context:(const VRORenderContext *)context {
    
}

@end
