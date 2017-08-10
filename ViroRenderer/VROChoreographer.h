//
//  VROChoreographer.h
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROChoreographer_h
#define VROChoreographer_h

#include <memory>
#include <vector>

class VROScene;
class VRODriver;
class VRORenderPass;
class VRORenderTarget;
class VRORenderContext;
class VROImagePostProcess;
class VROShaderProgram;

class VROChoreographer {
public:
    
    VROChoreographer(std::shared_ptr<VRODriver> driver);
    virtual ~VROChoreographer();
    
    virtual void render(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                        std::shared_ptr<VRODriver> &driver);
    
    void setBaseRenderPass(std::shared_ptr<VRORenderPass> pass) {
        _baseRenderPass = pass;
    }
    
    // This is only for testing currently
    void setUseBlitPass(bool useBlitPass) {
        _useBlit = useBlitPass;
    }
    
    /*
     Render targets need to be recreated when the viewport size is changed.
     */
    void setViewportSize(int width, int height);
    
private:
    
    bool _useBlit;
    
    /*
     The last recorded viewport size.
     */
    int _width, _height;
    
    /*
     Pass that renders the 3D scene to a render target.
     */
    std::shared_ptr<VRORenderPass> _baseRenderPass;
    
    /*
     Simple blitting post process.
     */
    std::shared_ptr<VROImagePostProcess> _blitPostProcess;
    
    /*
     Intermediate render target used when we are recording video or using post-process
     effects.
     */
    std::shared_ptr<VRORenderTarget> _blitTarget;
    
};

#endif /* VROChoreographer_h */
