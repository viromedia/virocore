//
//  VRODriverOpenGLiOSGVR.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/29/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRODriverOpenGLiOSGVR_h
#define VRODriverOpenGLiOSGVR_h

#include "VRODriverOpenGLiOS.h"
#include "VRODisplayOpenGLiOSGVR.h"

class VRODriverOpenGLiOSGVR : public VRODriverOpenGLiOS {
public:
    
    VRODriverOpenGLiOSGVR(GLKView *viewGL, EAGLContext *eaglContext, std::shared_ptr<gvr::AudioApi> gvrAudio) :
        VRODriverOpenGLiOS(viewGL, eaglContext, gvrAudio) {
    }
    virtual ~VRODriverOpenGLiOSGVR() { }
    
    /*
     On GVR the primary framebuffer (display) is tied to a GVR
     swapchain.
     */
    std::shared_ptr<VRORenderTarget> getDisplay() {
        if (!_display) {
            GLKView *viewGL = _viewGL;
            std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
            _display = std::make_shared<VRODisplayOpenGLiOSGVR>(viewGL, driver);
        }
        return _display;
    }
    
private:
    
};

#endif /* VRODriverOpenGLiOSGVR_h */
