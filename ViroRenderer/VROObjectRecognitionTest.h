//
//  VROObjectRecognitionTest.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/10/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef VROObjectRecognitionTest_h
#define VROObjectRecognitionTest_h

#include "VRORendererTest.h"
#include "VROARDeclarativeNode.h"
#include "VROSceneController.h"
#include "VRODefines.h"
#include "VROObjectRecognizer.h"
#include "VROARSession.h"

#if VRO_PLATFORM_IOS
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
        std::shared_ptr<VROARSession> arSession = _arScene->getARSession();
        arSession->setVisionModel(_objectRecognizer);
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
