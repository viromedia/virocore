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

class VRORenderer;

class VROSceneRendererCardboardOpenGL : public VROSceneRendererCardboard {
    
public:
    
    VROSceneRendererCardboardOpenGL(std::shared_ptr<VRORenderer> renderer);
    virtual ~VROSceneRendererCardboardOpenGL();
    
    virtual void initRenderer(GCSHeadTransform *headTransform);
    virtual void prepareFrame(GCSHeadTransform *headTransform);
    virtual void renderEye(GCSEye eye, GCSHeadTransform *headTransform);
    virtual void endFrame();
    
private:
    
    int _frame;
    std::shared_ptr<VRORenderer> _renderer;
    
};

#endif /* VROSceneRendererCardboardOpenGL_h */
