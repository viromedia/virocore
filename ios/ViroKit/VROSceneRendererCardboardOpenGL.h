//
//  VROSceneRendererCardboardOpenGL.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROSceneRendererCardboardOpenGL_h
#define VROSceneRendererCardboardOpenGL_h

#include "VROSceneRendererCardboard.h"
#include <memory>
#include "vr/gvr/capi/include/gvr_audio.h"

class VRORenderer;
class VRODriverOpenGL;

class VROSceneRendererCardboardOpenGL : public VROSceneRendererCardboard {
    
public:
    
    VROSceneRendererCardboardOpenGL(EAGLContext *context, std::shared_ptr<VRORenderer> renderer);
    virtual ~VROSceneRendererCardboardOpenGL();
    
    virtual void initRenderer(GVRHeadTransform *headTransform);
    virtual void prepareFrame(VROViewport viewport, VROFieldOfView fov, GVRHeadTransform *headTransform);
    virtual void renderEye(GVREye eye, GVRHeadTransform *headTransform);
    virtual void endFrame();
    
    void setSceneController(std::shared_ptr<VROSceneController> sceneController);
    void setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                            VROTimingFunctionType timingFunctionType);
    
    void setSuspended(bool suspended);
    void recenterTracking();

private:
    
    int _frame;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VRODriverOpenGL> _driver;
    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    bool _suspended;
    bool _recenterTracking;
    VROMatrix4f _baseRotation;

};

#endif /* VROSceneRendererCardboardOpenGL_h */
