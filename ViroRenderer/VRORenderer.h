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
    
    VRORenderer(std::shared_ptr<VRODevice> device, VRORenderContext *renderContext);
    virtual ~VRORenderer();
    
    void onOrientationChange(UIInterfaceOrientation orientation);
    
    virtual bool isVignetteEnabled() const = 0;
    virtual void setVignetteEnabled(bool vignetteEnabled) = 0;
    virtual bool isChromaticAberrationCorrectionEnabled() const = 0;
    virtual void setChromaticAberrationCorrectionEnabled(bool enabled) = 0;
    
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
    
protected:
    
    virtual void prepareFrame(const VRORenderContext &context) = 0;
    virtual void endFrame(const VRORenderContext &context) = 0;
    
    virtual void renderVRDistortion(const VRORenderContext &context) = 0;
    virtual void renderMonocular(const VRORenderContext &context) = 0;
    
    virtual void onEyesUpdated(VROEye *leftEye, VROEye *rightEye) = 0;
    
    void drawFrame(bool monocular);

private:
    
    VROMagnetSensor *_magnetSensor;
    VROHeadTracker *_headTracker;
    std::shared_ptr<VRODevice> _device;
    
    VROEye *_monocularEye;
    VROEye *_leftEye;
    VROEye *_rightEye;
        
    VRORenderContextMetal *_renderContext;
    
    VROScreenUIView *_HUD;
    
    bool _projectionChanged;
    bool _frameParamentersReady;
    bool _vrModeEnabled;
    bool _rendererInitialized;
    
    /*
     Internal representation of the camera.
     */
    std::shared_ptr<VROCameraMutable> _camera;
    
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
    
#pragma mark - Stereo renderer methods
    
    void renderEye(VROEyeType eyeType);
    
#pragma mark - Scene Loading
    
    bool processSceneTransition();
};

#endif /* VRORenderer_h */
