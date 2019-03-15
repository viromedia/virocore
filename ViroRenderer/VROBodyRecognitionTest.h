//
//  VROBodyRecognitionTest.h
//  ViroSample
//
//  Created by Raj Advani on 1/14/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROBodyRecognitionTest_h
#define VROBodyRecognitionTest_h

#include "VRORendererTest.h"
#include "VROARDeclarativeNode.h"
#include "VROSceneController.h"
#include "VRODefines.h"
#include "VROBodyTracker.h"
#include "VROARSession.h"

#if VRO_PLATFORM_IOS
#include "VROViewAR.h"
#import <UIKit/UIKit.h>

@interface VROBodyRecognitionDrawDelegate : NSObject<VRODebugDrawDelegate>
- (void)drawRect;
- (void)setLabels:(std::vector<NSString *>)labels positions:(std::vector<VROVector3f>)positions;
- (void)setBoxes:(std::vector<VROBoundingBox>)boxes;
- (void)setColors:(std::vector<UIColor *>)colors;
- (void)setConfidences:(std::vector<float>)confidences;
- (void)setDynamicCropBox:(CGRect)box;
- (void)setViewWidth:(int)width height:(int)height;
@end

#endif

class VROPoseFilter;

class VROBodyRecognitionTest : public VRORendererTest, public VROSceneController::VROSceneControllerDelegate,
                               public VROBodyTrackerDelegate, public std::enable_shared_from_this<VROBodyRecognitionTest> {
public:
    
    VROBodyRecognitionTest();
    virtual ~VROBodyRecognitionTest();
    
    void build(std::shared_ptr<VRORenderer> renderer,
               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
               std::shared_ptr<VRODriver> driver);
    
    std::shared_ptr<VRONode> getPointOfView() {
        return _pointOfView;
    }
    std::shared_ptr<VROSceneController> getSceneController() {
        return _sceneController;
    }
    
    virtual void onSceneWillAppear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
        
    }
                                   
    virtual void onSceneDidAppear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
        std::shared_ptr<VROARSession> arSession = _arScene->getARSession();
        arSession->setVisionModel(_bodyTracker);
    }
                                   
    virtual void onSceneWillDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
        
    }
    virtual void onSceneDidDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    }
    
    virtual void onBodyJointsFound(const VROPoseFrame &joints);
    
private:
    
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROARScene> _arScene;
    std::vector<std::shared_ptr<VRONode>> _bodyPointsSpheres;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROBodyTracker> _bodyTracker;
    
#if VRO_PLATFORM_IOS
    VROViewAR *_view;
    VROBodyRecognitionDrawDelegate *_drawDelegate;
#endif
    
};

#endif /* VROBodyRecognitionTest_h */
