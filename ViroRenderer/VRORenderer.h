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
#include "VROVector3f.h"
#include "VROQuaternion.h"
#include "VROMatrix4f.h"
#include "VRORenderDelegate.h"

class VROEye;
class VRODriverContext;
class VRODistortionRenderer;
class VRODevice;
class VROHeadTracker;
class VROCameraMutable;
class VROTimingFunction;
class VRORenderContext;
class VROViewport;
class VROFieldOfView;
enum class VROEyeType;
enum class VROTimingFunctionType;

@class VROScreenUIView; //TODO delete
@class VROView;
@class VROSceneController;

class VRORenderer {
    
public:
    
    VRORenderer();
    virtual ~VRORenderer();
        
    void setPosition(VROVector3f position);
    void setBaseRotation(VROQuaternion quaternion);
    float getWorldPerScreen(float distance, const VROFieldOfView &fov,
                            const VROViewport &viewport) const;
    
    void setSceneController(VROSceneController *sceneController);
    void setSceneController(VROSceneController *sceneController, bool animated);
    void setSceneController(VROSceneController *sceneController, float seconds, VROTimingFunctionType timingFunctionType);
    
    void setDelegate(id <VRORenderDelegate> delegate);
    
    void updateRenderViewSize(CGSize size);
    
    void prepareFrame(int frame, VROMatrix4f headRotation, VRODriverContext &driverContext);
    void renderEye(VROEyeType eye, VROMatrix4f eyeFromHeadMatrix, VROMatrix4f projectionMatrix,
                   const VRODriverContext &driverContext);
    void endFrame(const VRODriverContext &driverContext);

    void handleTap();
    VROScreenUIView *getHUD() {
        return _HUD;
    }
    
    VROSceneController *getSceneController() const {
        return _sceneController;
    }
    
private:
    
    bool _rendererInitialized;
    
    /*
     Maintains parameters used for scene rendering.
     */
    std::shared_ptr<VRORenderContext> _context;
    
    /*
     The screen-space view.
     */
    VROScreenUIView *_HUD;
    
    /*
     Internal representation of the camera.
     */
    std::shared_ptr<VROCameraMutable> _camera;
    
    /*
     Delegate receiving scene rendering updates.
     */
    id <VRORenderDelegate> _delegate; //TODO Make this not Obj-C, and weak ptr
        
#pragma mark - Scene and Scene Transitions
    
    VROSceneController *_sceneController;
    VROSceneController *_outgoingSceneController;

    bool _sceneTransitionActive;
    float _sceneTransitionDuration;
    float _sceneTransitionStartTime;
    std::unique_ptr<VROTimingFunction> _sceneTransitionTimingFunction;
    
    bool processSceneTransition();
    
#pragma mark - Scene Rendering
    
    void renderEye(VROEyeType eyeType, const VRODriverContext &driverContext);
    
};

#endif /* VRORenderer_h */
