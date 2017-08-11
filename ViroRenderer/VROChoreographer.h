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
#include <functional>

class VROScene;
class VRODriver;
class VROTexture;
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
    
    /*
     Enable or disable RTT. When RTT is enabled, the scene is rendered first
     to an offscreen buffer. Then it is flipped and blitted over to the provided
     texture. This enables other systems to process the rendered scene. The RTT
     callback is invoked each time a frame is rendered.
     
     Note the flip is required so that the texture appears right-side-up in the
     RTT texture.
     */
    void setRenderToTextureEnabled(bool enabled);
    void setRenderTexture(std::shared_ptr<VROTexture> texture);
    void setRenderToTextureCallback(std::function<void()> callback);
    
    /*
     Render targets need to be recreated when the viewport size is changed.
     */
    void setViewportSize(int width, int height);
    
private:
    
    /*
     RTT variables.
     */
    bool _renderToTexture;
    std::shared_ptr<VRORenderTarget> _renderToTextureTarget;
    std::shared_ptr<VROImagePostProcess> _renderToTexturePostProcess;
    std::function<void()> _renderToTextureCallback;
    
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
