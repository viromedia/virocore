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
#import <UIKit/UIKit.h>
#include "VROVector3f.h"
#include "VROQuaternion.h"
#include "VRORenderDelegate.h" //TODO Delete

class VROEye;
class VRODistortionRenderer;
class VROMagnetSensor;
class VRODevice;
class VROHeadTracker;
class VROCameraMutable;
class VROTimingFunction;
class VRORenderContext;
enum class VROEyeType;
enum class VROTimingFunctionType;

class VRORenderContextMetal; //TODO delegate
@class VROScreenUIView; //TODO delete
@class MTKView; //TODO delete
@class VROView;
@class VROSceneController;

class VRORenderer {
    
public:
    
    VRORenderer(VROView *view, VRORenderContext *renderContext);
    virtual ~VRORenderer();
    
    void onOrientationChange(UIInterfaceOrientation orientation);
    
    bool isVignetteEnabled() const;
    void setVignetteEnabled(bool vignetteEnabled);
    
    bool isChromaticAberrationCorrectionEnabled() const;
    void setChromaticAberrationCorrectionEnabled(bool enabled);
    
    float getVirtualEyeToScreenDistance() const;
    
    void setPosition(VROVector3f position);
    void setBaseRotation(VROQuaternion quaternion);
    
    float getWorldPerScreen(float distance) const;
    
    void setRenderDelegate(id <VRORenderDelegate> renderDelegate);
    
    void drawFrame();
    
    void setSceneController(VROSceneController *sceneController);
    void setSceneController(VROSceneController *sceneController, bool animated);
    void setSceneController(VROSceneController *sceneController, float seconds, VROTimingFunctionType timingFunctionType);
    
    void updateRenderViewSize(CGSize size);

    void handleTap();
    VRORenderContext *getRenderContext();
    VROScreenUIView *getHUD() {
        return _HUD;
    }
    
private:
    
    VROMagnetSensor *_magnetSensor;
    VROHeadTracker *_headTracker;
    VRODevice *_device;
    
    VROEye *_monocularEye;
    VROEye *_leftEye;
    VROEye *_rightEye;
    
    VRODistortionRenderer *_distortionRenderer;
    
    VRORenderContextMetal *_renderContext;
    dispatch_semaphore_t _inflight_semaphore;
    
    VROScreenUIView *_HUD;
    
    bool _projectionChanged;
    bool _frameParamentersReady;
    bool _vrModeEnabled;
    bool _rendererInitialized;
    
    /*
     Internal representation of the camera.
     */
    std::shared_ptr<VROCameraMutable> _camera;
    
    VROView *_view;
    id <VRORenderDelegate> _renderDelegate;
    
#pragma mark - Scene
    
    VROSceneController *_sceneController;
    VROSceneController *_outgoingSceneController;
    
    /*
     Scene transition variables.
     */
    float _sceneTransitionDuration;
    float _sceneTransitionStartTime;
    std::unique_ptr<VROTimingFunction> _sceneTransitionTimingFunction;
    
#pragma mark - View Computation

    void calculateFrameParameters();
    void updateMonocularEye();
    void updateLeftRightEyes();
    
    void renderVRDistortionInView(VROView *view, id <MTLCommandBuffer> commandBuffer);
    void renderMonocularInView(VROView *view, id <MTLCommandBuffer> commandBuffer);
    
#pragma mark - Stereo renderer methods
    
    void drawFrame(bool monocular);
    void renderEye(VROEyeType eyeType);
    
#pragma mark - Scene Loading
    
    bool processSceneTransition();
};

#endif /* VRORenderer_h */
