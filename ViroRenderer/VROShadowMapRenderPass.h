//
//  VROShadowMapRenderPass.h
//  ViroKit
//
//  Created by Raj Advani on 8/15/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROShadowMapRenderPass_h
#define VROShadowMapRenderPass_h

#include "VRORenderPass.h"
#include "VROMatrix4f.h"
#include "VROTree.h"

class VROPortal;
class VROLight;
class VROMaterial;
class VRORenderTarget;
class VROShaderModifier;

class VROShadowMapRenderPass : public VRORenderPass {
public:
    
    VROShadowMapRenderPass(const std::shared_ptr<VROLight> light, std::shared_ptr<VRODriver> driver);
    virtual ~VROShadowMapRenderPass();
    
    VRORenderPassInputOutput render(std::shared_ptr<VROScene> scene, VRORenderPassInputOutput &inputs,
                                    VRORenderContext *context, std::shared_ptr<VRODriver> &driver);
    
private:
    
    static std::shared_ptr<VROShaderModifier> getShadowDepthWritingModifier();
    
    /*
     Material used to render silhouettes of objects to the scene.
     */
    std::shared_ptr<VROMaterial> _silhouetteMaterial;
    
    /*
     The light casting the shadow.
     */
    const std::shared_ptr<VROLight> _light;
    
    /*
     Helper function for rendering. Performs depth-first rendering of portals, rendering
     the portal silhouettes to the stencil buffer on the way down, and the portal geometry
     and content on the way up.
     */
    void render(std::vector<tree<std::shared_ptr<VROPortal>>> &treeNodes,
                std::shared_ptr<VRORenderTarget> &target,
                const VRORenderContext &context,
                std::shared_ptr<VRODriver> &driver);
    
    VROMatrix4f computeLightProjectionMatrix() const;
    VROMatrix4f computeLightViewMatrix() const;
};

#endif /* VROShadowMapRenderPass_h */
