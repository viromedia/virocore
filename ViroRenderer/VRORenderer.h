//
//  VRORenderer.h
//  ViroRenderer
//
//  Created by Raj Advani on 4/5/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRORenderer_h
#define VRORenderer_h

#include <memory>
#include <vector>
#include <VROSceneController.h>
#include "VROVector3f.h"
#include "VROQuaternion.h"
#include "VROMatrix4f.h"
#include "VROViewport.h"
#include "VROFieldOfView.h"
#include "VROFrameSynchronizer.h"
#include "VROEventManager.h"

class VROEye;
class VRONode;
class VRODriver;
class VROCameraMutable;
class VROTimingFunction;
class VRORenderContext;
class VROFrameListener;
class VROReticle;
class VRORenderDelegateInternal;
enum class VROCameraRotationType;
enum class VROEyeType;
enum class VROTimingFunctionType;

static const float kZNear = 0.1;
static const float kZFar  = 100;

class VRORenderer {
    
public:
    
    VRORenderer();
    virtual ~VRORenderer();
        
    void setPosition(VROVector3f position);
    void setBaseRotation(VROQuaternion quaternion);
    void setCameraRotationType(VROCameraRotationType type);
    void setOrbitFocalPoint(VROVector3f focalPt);
    
    void setDelegate(std::shared_ptr<VRORenderDelegateInternal> delegate);
    void updateRenderViewSize(float width, float height);
    
#pragma mark - Scene Controllers
    
    void setSceneController(std::shared_ptr<VROSceneController> sceneController, VRODriver &driver);
    void setSceneController(std::shared_ptr<VROSceneController> sceneController, bool animated, VRODriver &driver);
    void setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                            VROTimingFunctionType timingFunctionType, VRODriver &driver);
    
#pragma mark - Render Loop
    
    void prepareFrame(int frame, VROViewport viewport, VROFieldOfView fov,
                      VROMatrix4f headRotation, VRODriver &driver);
    void renderEye(VROEyeType eye, VROMatrix4f eyeFromHeadMatrix, VROMatrix4f projectionMatrix,
                   VRODriver &driver);
    void endFrame(VRODriver &driver);
    
#pragma mark - Events
    
    std::shared_ptr<VROFrameSynchronizer> getFrameSynchronizer() {
        return _frameSynchronizer;
    }

     std::shared_ptr<VROEventManager> getEventManager(){
        return _eventManager;
     }

    std::shared_ptr<VROReticle> getReticle() {
        return _reticle;
    }
    
#pragma mark - VR Framework Specific
    // Some VR frameworks provide controls to allow the user to exit VR
    void requestExitVR();
    
private:

    bool _rendererInitialized;
    
    /*
     Manages per-frame listeners.
     */
    std::shared_ptr<VROFrameSynchronizer> _frameSynchronizer;
    
    /*
     Maintains parameters used for scene rendering.
     */
    std::shared_ptr<VRORenderContext> _context;

    /*
     Handles the processing and notification of input events (like ontap).
     */
    std::shared_ptr<VROEventManager> _eventManager;

    /*
     The reticle.
     */
    std::shared_ptr<VROReticle> _reticle;
    
    /*
     Internal representation of the camera.
     */
    std::shared_ptr<VROCameraMutable> _camera;
    
    /*
     Delegate receiving scene rendering updates.
     */
    std::weak_ptr<VRORenderDelegateInternal> _delegate;

#pragma mark - Scene and Scene Transitions
    
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROSceneController> _outgoingSceneController;

    bool _sceneTransitionActive;
    float _sceneTransitionDuration;
    float _sceneTransitionStartTime;
    std::unique_ptr<VROTimingFunction> _sceneTransitionTimingFunction;
    
    bool processSceneTransition();
    
#pragma mark - Scene Rendering
    
    void renderEye(VROEyeType eyeType, VRODriver &driver);
    
#pragma mark - Frame Listeners
    
    void notifyFrameStart();
    void notifyFrameEnd();
    
};

#endif /* VRORenderer_h */
