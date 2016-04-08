//
//  VRORendererMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 4/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRORendererMetal_h
#define VRORendererMetal_h

#include <Metal/Metal.h>
#include "VRORenderer.h"

@class VROView;

class VRORendererMetal : public VRORenderer {
    
public:
    
    VRORendererMetal(std::shared_ptr<VRODevice> device, VROView *view, VRORenderContext *context);
    virtual ~VRORendererMetal();
    
    void prepareFrame(const VRORenderContext &context);
    void endFrame(const VRORenderContext &context);
    
    void renderVRDistortion(const VRORenderContext &context);
    void renderMonocular(const VRORenderContext &context);
    
    bool isVignetteEnabled() const;
    void setVignetteEnabled(bool vignetteEnabled);
    bool isChromaticAberrationCorrectionEnabled() const;
    void setChromaticAberrationCorrectionEnabled(bool enabled);
    
    void onEyesUpdated(VROEye *leftEye, VROEye *rightEye);
    
private:
    
    __weak VROView *_view;
    VRODistortionRenderer *_distortionRenderer;

    dispatch_semaphore_t _inflight_semaphore;

};

#endif /* VRORendererMetal_h */
