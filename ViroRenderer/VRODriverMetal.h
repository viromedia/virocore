//
//  VRODriverMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 4/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODriverMetal_h
#define VRODriverMetal_h

#include <Metal/Metal.h>
#include "VRODriver.h"
#include "VRORenderDelegate.h"

class VRORenderer;
class VRODevice;
class VRODriverContext;
class VROEye;
class VROHeadTracker;
class VROViewport;
class VRODistortionRenderer;
@class VROView;

class VRODriverMetal : public VRODriver {
    
public:
    
    VRODriverMetal(id <MTLDevice> device, VROView *view);
    virtual ~VRODriverMetal();
    
    void driveFrame(int frame);
    
    bool isVignetteEnabled() const;
    void setVignetteEnabled(bool vignetteEnabled);
    bool isChromaticAberrationCorrectionEnabled() const;
    void setChromaticAberrationCorrectionEnabled(bool enabled);
    
    void onOrientationChange(UIInterfaceOrientation orientation);
    float getVirtualEyeToScreenDistance() const;
    
    VROViewport getViewport(VROEyeType eye);
    VROFieldOfView getFOV(VROEyeType eye);
    
    void setRenderer(std::shared_ptr<VRORenderer> renderer) {
        _renderer = renderer;
    }
    
private:
    
    bool _vrModeEnabled;
    bool _projectionChanged;
    
    VROEye *_monocularEye;
    VROEye *_leftEye;
    VROEye *_rightEye;
    
    std::shared_ptr<VRORenderer> _renderer;
    
    VROHeadTracker *_headTracker;
    std::shared_ptr<VRODevice> _device;
    std::shared_ptr<VRODriverContext> _context;
    
    __weak VROView *_view;
    VRODistortionRenderer *_distortionRenderer;
    
    dispatch_semaphore_t _inflight_semaphore;
    
#pragma mark - Rendering
    
    void renderVRDistortion(int frame, id <MTLCommandBuffer> commandBuffer);
    
#pragma mark - View Computation
    
    void calculateFrameParameters();
    void updateMonocularEye();
    void updateLeftRightEyes();
    VROEye *getEye(VROEyeType type);

};

#endif /* VRODriverMetal_h */
