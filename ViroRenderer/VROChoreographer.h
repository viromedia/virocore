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
#include "VROVector4f.h"

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
class VROPostProcessEffectFactory;
class VRORenderMetadata;
class VRORenderToTextureDelegate;
enum class VROPostProcessEffect;
enum class VROEyeType;

class VROChoreographer {
public:
    
    VROChoreographer(std::shared_ptr<VRODriver> driver);
    virtual ~VROChoreographer();
    
    virtual void render(VROEyeType eye,
                        std::shared_ptr<VROScene> scene,
                        std::shared_ptr<VROScene> outgoingScene,
                        const std::shared_ptr<VRORenderMetadata> &metadata,
                        VRORenderContext *context,
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

     TODO VIRO-2025: Remove these functions in favor of setting a VRORenderToTextureDelgateiOS.
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

    /*
     Retrieves the factory from which to enable/disable post processing effects applied
     in VROChoreographer::renderBasePass().
     */
    std::shared_ptr<VROPostProcessEffectFactory> getPostProcessEffectFactory();

    /*
     Sets a delegate that is invoked each time a frame has been rendered and passes it
     a reference to the final VRORenderTarget containing a texture representing the rendered scene.
     */
    void setRenderToTextureDelegate(std::shared_ptr<VRORenderToTextureDelegate> delegate);

    /*
     Updates the main set of render targets used in the rendering pipeline with the
     following clear color.
     */
    void setClearColor(VROVector4f color, std::shared_ptr<VRODriver> driver);

private:
    std::weak_ptr<VRODriver> _driver;

    /*
     True if the GPU supports multiple render targets.
     */
    bool _mrtEnabled;
    
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
     Render the 3D scene (and an optional outgoing scene), and perform post-processing.
     */
    void renderScene(std::shared_ptr<VROScene> scene,
                     std::shared_ptr<VROScene> outgoingScene,
                     const std::shared_ptr<VRORenderMetadata> &metadata,
                     VRORenderContext *context, std::shared_ptr<VRODriver> &driver);
    
#pragma mark - Render to Texture
    
    /*
     RTT variables.
     */
    bool _renderToTexture;
    std::shared_ptr<VRORenderTarget> _renderToTextureTarget;
    std::function<void()> _renderToTextureCallback;
    
    /*
     Render the given tone-mapped and gamma-corrected input to the
     video texture and display.
     */
    void renderToTextureAndDisplay(std::shared_ptr<VRORenderTarget> input,
                                   std::shared_ptr<VRODriver> driver);

    /*
     Delegate set by recorders to be notified of 'blitted' render targets containing texture
     representing the rendered scene that is needed for recording / screen capturing.
     */
    std::shared_ptr<VRORenderToTextureDelegate> _renderToTextureDelegate;
    
#pragma mark - Shadows
    
    /*
     True if shadow maps are enabled.
     */
    bool _renderShadows;
    
    /*
     The max supported shadow map size for the current device.
     */
    int _maxSupportedShadowMapSize;
    
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
     The size of the blur targets relative to the display. Smaller scale leads to
     less accurate but faster blur.
     */
    float _blurScaling;
    
    /*
     Render pass that iteratively performs Gaussian blur on the two blur targets.
     */
    std::shared_ptr<VROGaussianBlurRenderPass> _gaussianBlurPass;
    
    /*
     Additive blending post process for mapping the blur texture back onto the
     main texture.
     */
    std::shared_ptr<VROImagePostProcess> _additiveBlendPostProcess;

#pragma mark - Additional Post-Process Effects

    /*
     Factory that coordinates the creation and application of post processing effects.
     */
    std::shared_ptr<VROPostProcessEffectFactory> _postProcessEffectFactory;

    /*
     Intermediate target used for post-processing effects.
     */
    std::shared_ptr<VRORenderTarget> _postProcessTarget;
};

#endif /* VROChoreographer_h */
