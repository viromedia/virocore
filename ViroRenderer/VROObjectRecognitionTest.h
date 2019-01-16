//
//  VROObjectRecognitionTest.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/10/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROObjectRecognitionTest_h
#define VROObjectRecognitionTest_h

#include "VRORendererTest.h"
#include "VROARDeclarativeNode.h"
#include "VROSceneController.h"
#include "VRODefines.h"
#include "VROObjectRecognizer.h"

#if VRO_PLATFORM_IOS
#include "VROARSessioniOS.h"
#import "VROViewAR.h"
#import <UIKit/UIKit.h>

@interface VRORecognitionDrawDelegate : NSObject<VRODebugDrawDelegate>
- (void)drawRect;
- (void)setLabels:(std::vector<NSString *>)labels positions:(std::vector<VROVector3f>)positions;
- (void)setBoxes:(std::vector<VROBoundingBox>)boxes;
- (void)setColors:(std::vector<UIColor *>)colors;
@end

#endif

class VROObjectRecognitionTest : public VRORendererTest, public VROSceneController::VROSceneControllerDelegate,
                           public VROObjectRecognizerDelegate,
                           public std::enable_shared_from_this<VROObjectRecognitionTest> {
public:
    
    VROObjectRecognitionTest();
    virtual ~VROObjectRecognitionTest();
    
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
#if VRO_PLATFORM_IOS
        std::shared_ptr<VROARSession> arSession = _arScene->getARSession();
        std::shared_ptr<VROARSessioniOS> arSessioniOS = std::dynamic_pointer_cast<VROARSessioniOS>(arSession);
        arSessioniOS->setObjectRecognizer(_objectRecognizer);
#endif
    }
    virtual void onSceneWillDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
        
    }
    virtual void onSceneDidDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    }
    
    virtual void onObjectsFound(const std::map<std::string, std::vector<VRORecognizedObject>> &joints);
    
private:
    
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROARScene> _arScene;
    std::vector<std::shared_ptr<VRONode>> _bodyPointsSpheres;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROObjectRecognizer> _objectRecognizer;
                               
#if VRO_PLATFORM_IOS
    VROViewAR *_view;
    VRORecognitionDrawDelegate *_drawDelegate;
#endif
    
};

#endif /* VROObjectRecognitionTest_hpp */
