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
#include <map>
#include <vector>
#include <functional>

class VROScene;
class VRODriver;
class VROLight;
class VROViewport;
class VROTexture;
class VRORenderPass;
class VRORenderTarget;
class VRORenderContext;
class VROImagePostProcess;
class VROShaderProgram;
class VROShadowMapRenderPass;
enum class VROEyeType;

class VROChoreographer {
public:
    
    VROChoreographer(std::shared_ptr<VRODriver> driver);
    virtual ~VROChoreographer();
    
    virtual void render(VROEyeType eye, std::shared_ptr<VROScene> scene, VRORenderContext *context,
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
     Render targets need to be recreated when the viewport size is changed. They
     also need to be able to set their viewport when bound.
     */
    void setViewport(VROViewport viewport, std::shared_ptr<VRODriver> &driver);
    
private:
    
    /*
     RTT variables.
     */
    bool _renderToTexture;
    std::shared_ptr<VRORenderTarget> _renderToTextureTarget;
    std::shared_ptr<VROImagePostProcess> _renderToTexturePostProcess;
    std::function<void()> _renderToTextureCallback;
    
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
    
    /*
     The shadow passes for creating the depth maps for each light.
     */
    std::map<std::shared_ptr<VROLight>, std::shared_ptr<VROShadowMapRenderPass>> _shadowPasses;
    
    /*
     True if shadow maps are enabled.
     */
    bool _renderShadows;
    
    /*
     Initialize the various render targets.
     */
    void initTargets(std::shared_ptr<VRODriver> driver);
    
    /*
     Render shadows to the shadow maps for each shadow casting light.
     */
    void renderShadowPasses(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                            std::shared_ptr<VRODriver> &driver);
    
    /*
     Render the base pass, which renders the 3D scene to the first render target.
     */
    void renderBasePass(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                        std::shared_ptr<VRODriver> &driver);
    
};

#endif /* VROChoreographer_h */
