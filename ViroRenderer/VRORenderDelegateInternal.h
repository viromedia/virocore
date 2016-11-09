//
//  VRORenderDelegateInternal.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/6/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRORenderDelegateInternal_h
#define VRORenderDelegateInternal_h

enum class VROEyeType;
class VRORenderContext;
class VROVector3f;

class VRORenderDelegateInternal {
    
public:
    
    VRORenderDelegateInternal() {}
    virtual ~VRORenderDelegateInternal() {}
    
    virtual void setupRendererWithDriver(VRODriver *driver) = 0;
    virtual void shutdownRenderer() = 0;
    virtual void renderViewDidChangeSize(float width, float height, VRORenderContext *context) = 0;
    
    virtual void willRenderEye(VROEyeType eye, const VRORenderContext *context) = 0;
    virtual void didRenderEye(VROEyeType eye, const VRORenderContext *context) = 0;
    virtual void reticleTapped(VROVector3f ray, const VRORenderContext *context) = 0;
    virtual void userDidRequestExitVR() = 0;
    
};

#endif /* VRORenderDelegateInternal_h */
