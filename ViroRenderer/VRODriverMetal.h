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
class VRODriverContextMetal;
class VROEye;
class VROHeadTracker;
class VROViewport;
class VRODistortionRenderer;
@class VROViewMetal;

class VRODriverMetal : public VRODriver, public std::enable_shared_from_this<VRODriverMetal> {
    
public:
    
    VRODriverMetal(std::shared_ptr<VRORenderer> renderer);
    virtual ~VRODriverMetal();
    
    void driveFrame();
    
    UIView *getRenderingView();
    void onOrientationChange(UIInterfaceOrientation orientation);
    
    VROViewport getViewport(VROEyeType eye);
    VROFieldOfView getFOV(VROEyeType eye);
    
private:
    
    int _frame;
    bool _vrModeEnabled;
    bool _projectionChanged;
    
    VROViewMetal *_view;
    VROEye *_monocularEye;
    VROEye *_leftEye;
    VROEye *_rightEye;
    
    std::shared_ptr<VRORenderer> _renderer;
    
    VROHeadTracker *_headTracker;
    std::shared_ptr<VRODevice> _device;
    std::shared_ptr<VRODriverContextMetal> _context;
    
#pragma mark - Rendering

    VRODistortionRenderer *_distortionRenderer;
    dispatch_semaphore_t _inflight_semaphore;
    
    void renderVRDistortion(int frame, id <MTLCommandBuffer> commandBuffer);
    
#pragma mark - View Computation
    
    float getVirtualEyeToScreenDistance() const;
    void calculateFrameParameters();
    void updateMonocularEye();
    void updateLeftRightEyes();
    VROEye *getEye(VROEyeType type);

};

#endif /* VRODriverMetal_h */
