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
#include <functional>
#include <memory>

class VRONode;
class VROPortal;
class VROLight;
class VROMaterial;
class VRORenderTarget;
class VRORenderContext;
class VROShaderModifier;
enum class VROSilhouetteFilter;

/*
 Setting this to true enable shadow map debug mode. This will:
 
 1. Disable using texture arrays, which limits us to one shadow but
    enables us to use the Xcode OpenGL snapshot (the snapshot crashes if
    used with texture arrays).
 
 2. Draw the shadow frustum using the VROPencil.
 */
const bool kDebugShadowMaps = false;

/*
 The maximum number of shadow maps allowed. Note that while kMaxLights
 determines the number of lights that can be used *at one time*, this
 determines the maximum number of shadow maps that can be created per
 frame. That is, if we have 48 lights, only kMaxShadowMaps of them will be
 able to cast a shadow in a given frame.
 */
const int kMaxShadowMaps = 32;

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
    std::shared_ptr<VROMaterial> _silhouetteStaticMaterial;
    std::shared_ptr<VROMaterial> _silhouetteSkeletalMaterial;
    
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
                std::shared_ptr<VROMaterial> material,
                std::function<bool(const VRONode &)> filter,
                const VRORenderContext &context,
                std::shared_ptr<VRODriver> &driver);
    
    VROMatrix4f computeLightProjectionMatrix() const;
    VROMatrix4f computeLightViewMatrix() const;
};

#endif /* VROShadowMapRenderPass_h */
