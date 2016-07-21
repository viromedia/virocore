//
//  VROView.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "VRORenderDelegate.h"
#import "VROScreenUIView.h"
#import <memory>

@class VROSceneController;
class VROScene;
class VROFrameSynchronizer;
enum class VROTimingFunctionType;

enum class VROCameraRotationType {
    Standard,
    Orbit
};

@protocol VROView <NSObject>

@required

@property (nonatomic, unsafe_unretained) IBOutlet id <VRORenderDelegate> renderDelegate;
@property (nonatomic, readonly) VROScreenUIView *HUD;
@property (readwrite, nonatomic) VROSceneController *sceneController;

- (void)setSceneController:(VROSceneController *)sceneController animated:(BOOL)animated;
- (void)setSceneController:(VROSceneController *)sceneController duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType;

- (void)setPosition:(VROVector3f)position;
- (void)setBaseRotation:(VROQuaternion)rotation;
- (void)setCameraRotationType:(VROCameraRotationType)type;
- (void)setOrbitFocalPoint:(VROVector3f)focalPt;

- (float)worldPerScreenAtDepth:(float)distance;

- (std::shared_ptr<VROFrameSynchronizer>)frameSynchronizer;

@end
