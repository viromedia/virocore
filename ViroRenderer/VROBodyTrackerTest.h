//
//  VROBodyTrackerTest.h
//  ViroSample
//
//  Created by vik.advani on 9/7/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROBodyTrackerTest_h
#define VROBodyTrackerTest_h

#include "VRORendererTest.h"
#include "VROARDeclarativeNode.h"
#include "VROSceneController.h"
#include "VRODefines.h"
#include "VROBodyTracker.h"
#include "VROEventDelegate.h"
#include "VROBodyTrackerController.h"


#if VRO_PLATFORM_IOS
#include "VROAnimBodyDataiOS.h"
#include "VROARSessioniOS.h"
#import <UIKit/UIKit.h>
#endif

class VROBodyTrackerTest : public VRORendererTest,
                           public VROEventDelegate,
                           public VROSceneController::VROSceneControllerDelegate,
                           public VROBodyTrackerControllerDelegate,
                           public VROFrameListener,
                           public std::enable_shared_from_this<VROBodyTrackerTest> {
public:
    
    VROBodyTrackerTest();
    virtual ~VROBodyTrackerTest();
    
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
        createNewBodyTracker();
    }
    virtual void onSceneWillDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {

    }
    virtual void onSceneDidDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    }

    virtual void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position) {
        if (VROStringUtil::strcmpinsensitive(node->getTag(), "Recalibrate")) {
            _bodyMLController->startCalibration();
        } else if (VROStringUtil::strcmpinsensitive(node->getTag(), "Rebind")) {
            if (_modelNodeNinja1->getParentNode() == nullptr) {
                _modelNodeNinja2->removeFromParentNode();
                _gltfNodeContainer->addChildNode(_modelNodeNinja1);
                _bodyMLController->bindModel(_modelNodeNinja1);
            } else {
                _modelNodeNinja1->removeFromParentNode();
                _gltfNodeContainer->addChildNode(_modelNodeNinja2);
                _bodyMLController->bindModel(_modelNodeNinja2);
            }
        } else if (VROStringUtil::strcmpinsensitive(node->getTag(), "AutoCalibrate")) {
            _loadNewConfig = true;
        } else if(VROStringUtil::strcmpinsensitive(node->getTag(), "Record") && (clickState == ClickDown)) {
            if(!_bodyMLController->isRecording()) {
                pinfo("Starting recording");
                _bodyMLController->startRecording();
            }
            else {
                std::string jsonStringData = _bodyMLController->stopRecording();
                pinfo("Stopping recording.");
                // stop body tracking
                _bodyTracker->stopBodyTracking();
                // setDelagete to nil is currently only way to stop body tracking.
                _bodyTracker->setDelegate(NULL);
                _bodyPlaybackController->bindModel(_modelNodeNinja1);

                _modelNodeNinja1->setScale(VROVector3f(0.05f, 0.05f, 0.05f));
                _modelNodeNinja1->setPosition(VROVector3f(0.0f,0.0f, -.2f));
                if (_bodyPlayer) {
                    _bodyPlayer->loadAnimation(jsonStringData);
                    _bodyPlayer->setLooping(true);
                    _bodyPlayer->start();
                }
            }
        } else if (VROStringUtil::strcmpinsensitive(node->getTag(), "Model") && (clickState == ClickDown)){
            pwarn("VROBodyTrackerTest : you have clicked the model.");
        }
    }

    void onModelLoaded(std::shared_ptr<VRONode> node);
    void onBodyTrackStateUpdate(VROBodyTrackedState state);
    void renderDebugSkeletal(std::shared_ptr<VROPencil> pencil, int jointIndex);
    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);
    void createNewBodyTracker();
    void createNewBodyController();
private:
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROARScene> _arScene;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRONode> _modelNodeNinja1;
    std::shared_ptr<VRONode> _modelNodeNinja2;
    std::shared_ptr<VRONode> _gltfNodeContainer;
    std::shared_ptr<VRONode> _modelNode;
    std::shared_ptr<VROBodyTracker> _bodyTracker;
    std::shared_ptr<VROBodyPlayer> _bodyPlayer;
    std::shared_ptr<VROBodyTrackerController> _bodyMLController;
    std::shared_ptr<VROBodyTrackerController> _bodyPlaybackController;
    std::shared_ptr<VROText> _trackingStateText;
    std::shared_ptr<VROSkinner> _skinner;
    std::shared_ptr<VRODriver> _driver;
    bool _loadNewConfig = false;
    std::shared_ptr<VRONode> createTriggerBox(VROVector3f pos, VROVector4f color, std::string tag);
    std::shared_ptr<VROFrameSynchronizer> _frameSynchronizer;
};

#endif /* VROBodyTrackerTest_h  */
