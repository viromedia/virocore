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
class VROMaterial;
class VROShaderModifier;

class VROShadowMapRenderPass : public VRORenderPass {
public:
    
    // TODO Each pass should be associated with a light
    VROShadowMapRenderPass();
    virtual ~VROShadowMapRenderPass();
    
    VRORenderPassInputOutput render(std::shared_ptr<VROScene> scene, VRORenderPassInputOutput &inputs,
                                    VRORenderContext *context, std::shared_ptr<VRODriver> &driver);
    
    /*
     Get the view, projection, and bias matrices used to transform any point in world
     space into its corresponding texcoord in the light's shadow depth map.
     This includes the 'bias' matrix that translates the coordinates from
     homogenuous coordinates [-1,1] into texture sampling coords [0,1].
     */
    VROMatrix4f getShadowViewMatrix() const {
        return _shadowView;
    }
    VROMatrix4f getShadowProjectionMatrix() const {
        return _shadowProjection;
    }
    
private:
    
    static std::shared_ptr<VROShaderModifier> getShadowDepthWritingModifier();
    
    /*
     Material used to render silhouettes of objects to the scene.
     */
    std::shared_ptr<VROMaterial> _silhouetteMaterial;
    
    /*
     The projection and view matrices computed during the last render of
     the shadow map.
     */
    VROMatrix4f _shadowView;
    VROMatrix4f _shadowProjection;
    
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
