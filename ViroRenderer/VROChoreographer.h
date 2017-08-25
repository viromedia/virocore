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
class VROToneMappingRenderPass;
class VROGaussianBlurRenderPass;
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
    
    /*
     Retrieve the configurable tone mapping pass.
     */
    std::shared_ptr<VROToneMappingRenderPass> getToneMapping();
    
private:
    
    std::weak_ptr<VRODriver> _driver;
    
#pragma mark - Render Scene
    
    /*
     Pass that renders the 3D scene to a render target.
     */
    std::shared_ptr<VRORenderPass> _baseRenderPass;
    
    /*
     Simple blitting post process.
     */
    std::shared_ptr<VROImagePostProcess> _blitPostProcess;
    
    /*
     Intermediate render target used for recording video, and other
     post processes.
     */
    std::shared_ptr<VRORenderTarget> _blitTarget;
    
    /*
     Initialize the various render targets.
     */
    void initTargets(std::shared_ptr<VRODriver> driver);
    
    /*
     Render the base pass, which renders the 3D scene to the first render target.
     */
    void renderBasePass(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                        std::shared_ptr<VRODriver> &driver);
    
#pragma mark - Render to Texture
    
    /*
     RTT variables.
     */
    bool _renderToTexture;
    std::shared_ptr<VRORenderTarget> _renderToTextureTarget;
    std::shared_ptr<VROImagePostProcess> _renderToTexturePostProcess;
    std::function<void()> _renderToTextureCallback;
    
    /*
     Render the given tone-mapped and gamma-corrected input to the
     video texture and display.
     */
    void renderToTextureAndDisplay(std::shared_ptr<VRORenderTarget> input,
                                   std::shared_ptr<VRODriver> driver);
    
#pragma mark - Shadows
    
    /*
     True if shadow maps are enabled.
     */
    bool _renderShadows;
    
    /*
     The render target for the shadow passes. This target uses a depth texture array
     to capture shadow maps for all lights.
     */
    std::shared_ptr<VRORenderTarget> _shadowTarget;
    
    /*
     The shadow passes for creating the depth maps for each light.
     */
    std::map<std::shared_ptr<VROLight>, std::shared_ptr<VROShadowMapRenderPass>> _shadowPasses;
    
    /*
     Render shadows to the shadow maps for each shadow casting light.
     */
    void renderShadowPasses(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                            std::shared_ptr<VRODriver> &driver);
    
#pragma mark - HDR
    
    /*
     True if HDR rendering is enabled. When HDR rendering is enabled, the scene
     is rendered to a floating point texture, then a tone-mapping algorithm is
     applied to preserve details in both bright and dark regions of the
     scene.
     */
    bool _renderHDR;
    
    /*
     Floating point target for initially rendering the scene.
     */
    std::shared_ptr<VRORenderTarget> _hdrTarget;
    
    /*
     Tone mapping render pass to render the floating point scene in RGB.
     */
    std::shared_ptr<VROToneMappingRenderPass> _toneMappingPass;
    
    /*
     Initialize the render pass and targets for HDR rendering.
     */
    void initHDR(std::shared_ptr<VRODriver> driver);
    
#pragma mark - Bloom
    
    /*
     Enable or disable bloom rendering. When Bloom is enabled, an additional
     color buffer is bound that receives bright colors via a special bloom shader
     modifier. This buffer is blurred and added back to the scene.
     */
    bool _renderBloom;
    
    /*
     Render targets for ping-ponging the blur operation.
     */
    std::shared_ptr<VRORenderTarget> _blurTargetA;
    std::shared_ptr<VRORenderTarget> _blurTargetB;
    
    /*
     Render pass that iteratively performs Gaussian blur on the two blur targets.
     */
    std::shared_ptr<VROGaussianBlurRenderPass> _gaussianBlurPass;
    
};

#endif /* VROChoreographer_h */
