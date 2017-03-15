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
#include "VROSceneController.h"
#include "VROVector3f.h"
#include "VROQuaternion.h"
#include "VROMatrix4f.h"
#include "VROViewport.h"
#include "VROFieldOfView.h"
#include "VROFrameSynchronizer.h"
#include "VROInputControllerBase.h"

class VROEye;
class VRONode;
class VRODriver;
class VRONodeCamera;
class VROTimingFunction;
class VRORenderContext;
class VROFrameListener;
class VROReticle;
class VRORenderDelegateInternal;
enum class VROCameraRotationType;
enum class VROEyeType;
enum class VROTimingFunctionType;

static const float kZNear = 0.25;
static const float kZFar  = 50;

class VRORenderer {
    
public:

    VRORenderer(std::shared_ptr<VROInputControllerBase> inputController);
    virtual ~VRORenderer();
    
    void setPointOfView(std::shared_ptr<VRONode> node);
    void setDelegate(std::shared_ptr<VRORenderDelegateInternal> delegate);
    void updateRenderViewSize(float width, float height);
    
#pragma mark - Scene Controllers
    
    void setSceneController(std::shared_ptr<VROSceneController> sceneController, VRODriver &driver);
    void setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                            VROTimingFunctionType timingFunctionType, VRODriver &driver);
    
#pragma mark - Render Loop
    
    void prepareFrame(int frame, VROViewport viewport, VROFieldOfView fov,
                      VROMatrix4f headRotation, VRODriver &driver);
    void renderEye(VROEyeType eye, VROMatrix4f eyeFromHeadMatrix, VROMatrix4f projectionMatrix,
                   VRODriver &driver);
    void endFrame(VRODriver &driver);
    
#pragma mark - Integration
    
    std::shared_ptr<VROFrameSynchronizer> getFrameSynchronizer() {
        return _frameSynchronizer;
    }

    std::shared_ptr<VROInputControllerBase> getInputController(){
        return _inputController;
    }

    std::shared_ptr<VRORenderContext> getRenderContext() {
        return _context;
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
     Controller used for handling all input events.
     */
    std::shared_ptr<VROInputControllerBase> _inputController;

    /*
<<<<<<< HEAD
     The reticle.
     */
    std::shared_ptr<VROReticle> _reticle;
    
    /*
     The node that owns the VRONodeCamera that will determine the point of
     view from which we display the scene.
=======
     Internal representation of the camera.
>>>>>>> 139bdac... VIRO-696: DayDream Controller Integration Part 1
     */
    std::shared_ptr<VRONode> _pointOfView;
    
    /*
     Delegate receiving scene rendering updates.
     */
    std::weak_ptr<VRORenderDelegateInternal> _delegate;

#pragma mark - Scene and Scene Transitions
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROSceneController> _outgoingSceneController;

#pragma mark - Scene Rendering
    
    void renderEye(VROEyeType eyeType, VRODriver &driver);
    
#pragma mark - Frame Listeners
    
    void notifyFrameStart();
    void notifyFrameEnd();
    
};

#endif /* VRORenderer_h */
