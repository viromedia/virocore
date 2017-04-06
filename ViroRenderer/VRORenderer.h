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
class VRODebugHUD;
class VRONodeCamera;
class VROTimingFunction;
class VRORenderContext;
class VROFrameListener;
class VROReticle;
class VRORenderDelegateInternal;
class VROFrameScheduler;
enum class VROCameraRotationType;
enum class VROEyeType;
enum class VROTimingFunctionType;

static const float kZNear = 0.25;
static const float kZFar  = 50;
static const int kFPSMaxSamples = 100;

class VRORenderer {
    
public:

    VRORenderer(std::shared_ptr<VROInputControllerBase> inputController);
    virtual ~VRORenderer();
    
    void setPointOfView(std::shared_ptr<VRONode> node);
    void setDelegate(std::shared_ptr<VRORenderDelegateInternal> delegate);
    void updateRenderViewSize(float width, float height);
    void setDebugHUDEnabled(bool enabled);
    
#pragma mark - Scene Controllers
    
    void setSceneController(std::shared_ptr<VROSceneController> sceneController, std::shared_ptr<VRODriver> driver);
    void setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                            VROTimingFunctionType timingFunctionType, std::shared_ptr<VRODriver> driver);
    
#pragma mark - Render Loop
    
    void prepareFrame(int frame, VROViewport viewport, VROFieldOfView fov,
                      VROMatrix4f headRotation, std::shared_ptr<VRODriver> driver);
    void renderEye(VROEyeType eye, VROMatrix4f eyeFromHeadMatrix, VROMatrix4f projectionMatrix,
                   std::shared_ptr<VRODriver> driver);
    void endFrame(std::shared_ptr<VRODriver> driver);
    
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
     The node that owns the VRONodeCamera that will determine the point of
     view from which we display the scene.
     */
    std::shared_ptr<VRONode> _pointOfView;
    
    /*
     Delegate receiving scene rendering updates.
     */
    std::weak_ptr<VRORenderDelegateInternal> _delegate;
    
    /*
     HUD for displaying debug information.
     */
    std::unique_ptr<VRODebugHUD> _debugHUD;

    /*
     Invoked on the rendering thread to perform thread-specific initialization.
     */
    void initRenderer(std::shared_ptr<VRODriver> driver);
    
#pragma mark - FPS Computation
    
    /*
     Variables for FPS computation. Array of samples taken, index of
     next sample, and sum of samples so far.
     */
    int _fpsTickIndex = 0;
    uint64_t _fpsTickSum = 0;
    uint64_t _fpsTickArray[kFPSMaxSamples];
    
    /*
     FPS is measured in ticks, which are the number of nanoseconds
     between consecurive calls to prepareFrame().
     */
    uint64_t _nanosecondsLastFrame;
    
    /*
     FPS is computed via moving average. Updates the moving average
     with the last tick.
     */
    void updateFPS(uint64_t newTick);
    double getFPS() const;
    
#pragma mark - Process scheduling
    
    /*
     Milliseconds per frame target; derived from desired FPS.
     */
    double _mpfTarget;
    
    /*
     The time the current frame started and ended its display functions.
     */
    double _frameStartTime;
    double _frameEndTime;
    
    /*
     Schedules tasks to be run on the rendering thread when remaining
     frame time allows.
     */
    std::shared_ptr<VROFrameScheduler> _frameScheduler;

#pragma mark - Scene and Scene Transitions
    
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROSceneController> _outgoingSceneController;

#pragma mark - Scene Rendering
    
    void renderEye(VROEyeType eyeType, std::shared_ptr<VRODriver> driver);
    
#pragma mark - Frame Listeners
    
    void notifyFrameStart();
    void notifyFrameEnd();
    
};

#endif /* VRORenderer_h */
