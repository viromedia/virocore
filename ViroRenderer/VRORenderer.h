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
#include "VROPostProcessEffectFactory.h"

class VROEye;
class VRONode;
class VRODriver;
class VRODebugHUD;
class VRONodeCamera;
class VROTimingFunction;
class VRORenderContext;
class VROFrameListener;
class VRORenderDelegateInternal;
class VROFrameScheduler;
class VROChoreographer;
enum class VROCameraRotationType;
enum class VROEyeType;
enum class VROTimingFunctionType;

// The (fixed) near clipping plane
static const float kZNear = 0.01;

// The far clipping plane to use if no scene is loaded; also serves as a
// mininum far clipping plane
static const float kZFar  = 50;

// Multiplified by the furthest out object in the active scene to determine
// the FCP.
static const float kZFarMultiplier = 1.15;

// Number of samples to collect when computing FPS
static const int kFPSMaxSamples = 100;

class VRORenderer {

public:

    VRORenderer(std::shared_ptr<VROInputControllerBase> inputController);

    virtual ~VRORenderer();

    /*
     Set the node that contains the camera through which we will view the
     scene.
     */
    void setPointOfView(std::shared_ptr<VRONode> node);

    /*
     Set the delegate that can be used to respond to renderer state changes.
     */
    void setDelegate(std::shared_ptr<VRORenderDelegateInternal> delegate);

    /*
     Set to true to enable the debug HUD, which draws frame data directly
     to the screen.
     */
    void setDebugHUDEnabled(bool enabled);

    /*
     Get the VROChoreographer, which can be used to customize the rendering
     technique.
     */
    const std::shared_ptr<VROChoreographer> getChoreographer() const;

    void updateRenderViewSize(float width, float height);

#pragma mark - Viewport and FOV

    /*
     Get the field of view to use for a viewport of the given size. This function
     is generally only used in mono-rendering VR (e.g. not stereo VR). In
     stereo VR, the FOV is typically determined by the platform. In MonoVR,
     however, we have more control and need to support a wider array of viewport 
     sizes. We use a default horizontal FOV for this method to determine the 
     vertical FOV.
     */
    static VROFieldOfView computeMonoFOV(int viewportWidth, int viewportHeight);

    /*
     Get the field of view (vertical and horizontal) to use to render a viewport
     of the given size, given a horizontal FOV of the given degrees. This function
     is generally only used in AR, where we are given a camera horizontal FOV and
     need to create a rendering horizontal and vertical FOV to match said camera.
     
     Note the given horizontalFOVDegrees is the degrees from edge to edge of the 
     frustum.
     */
    static VROFieldOfView computeFOV(float horizontalFOVDegrees, int viewportWidth,
                                     int viewportHeight);

    /*
     Get the far clipping plane, as computed during the last prepareFrame().
     */
    float getFarClippingPlane() const;

#pragma mark - Scene Controllers

    void setSceneController(std::shared_ptr<VROSceneController> sceneController,
                            std::shared_ptr<VRODriver> driver);

    void setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                            VROTimingFunctionType timingFunctionType,
                            std::shared_ptr<VRODriver> driver);

    /*
     Applies scene specific post processing configuration effects to the renderer if we
     haven't yet done so already (since the scene had appeared), or if it had changed.
     */
    void updateSceneEffects(std::shared_ptr<VRODriver> driver, std::shared_ptr<VROScene> scene);
#pragma mark - Render Loop
    
    /*
     Prepare to render the next frame. This computes transforms and physics, processes
     animations, updates visibility, and sorts visible objects so they can be rendered
     swiftly for each eye.
     
     Note the projection matrix used here is used to construct the visibility frustum,
     not for rendering.
     */
    void prepareFrame(int frame, VROViewport viewport, VROFieldOfView fov,
                      VROMatrix4f headRotation, VROMatrix4f projection, std::shared_ptr<VRODriver> driver);
    
    /*
     Render the designated eye.
     */
    void renderEye(VROEyeType eye, VROMatrix4f eyeFromHeadMatrix, VROMatrix4f projection,
                   VROViewport viewport, std::shared_ptr<VRODriver> driver);
     
    /*
     Performs end-frame cleanup.
     */
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
     Maintains parameters used for scene rendering.
     */
    std::shared_ptr<VRORenderContext> _context;
    
    /*
     Sequences the render passes and targets. Defines the overall rendering
     technique.
     */
    std::shared_ptr<VROChoreographer> _choreographer;

    /*
     Controller used for handling all input events.
     */
    std::shared_ptr<VROInputControllerBase> _inputController;
    
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
  
#pragma mark - [Private] Camera and Visibility
    
    /*
     The node that owns the VRONodeCamera that will determine the point of
     view from which we display the scene.
     */
    std::shared_ptr<VRONode> _pointOfView;
  
    VROCamera updateCamera(const VROViewport &viewport, const VROFieldOfView &fov,
                           const VROMatrix4f &headRotation, const VROMatrix4f &projection);
    
#pragma mark - [Private] FPS Computation
    
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
    
#pragma mark - [Private] Process scheduling
    
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

#pragma mark - [Private] Scene and Scene Transitions
    
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VROSceneController> _outgoingSceneController;

#pragma mark - [Private] Scene Rendering
    
    void renderEye(VROEyeType eyeType, std::shared_ptr<VRODriver> driver);

#pragma mark - [Private] Frame Listeners
    
    /*
     Manages per-frame listeners.
     */
    std::shared_ptr<VROFrameSynchronizer> _frameSynchronizer;
    
    void notifyFrameStart();
    void notifyFrameEnd();
    
};

#endif /* VRORenderer_h */
