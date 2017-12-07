//
//  VROPortalTreeRenderPass.h
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROPortalTreeRenderPass_h
#define VROPortalTreeRenderPass_h

#include "VRORenderPass.h"
#include "VROTree.h"

class VRODriver;
class VROPortal;
class VROMaterial;

class VROPortalTreeRenderPass : public VRORenderPass {
public:
    
    VROPortalTreeRenderPass();
    virtual ~VROPortalTreeRenderPass();
    
    VRORenderPassInputOutput render(std::shared_ptr<VROScene> scene,
                                    std::shared_ptr<VROScene> outgoingScene,
                                    VRORenderPassInputOutput &inputs,
                                    VRORenderContext *context, std::shared_ptr<VRODriver> &driver);
    
private:
    
    /*
     Material used to render silhouettes of objects to the scene.
     */
    std::shared_ptr<VROMaterial> _silhouetteMaterial;
    
    /*
     Helper function for rendering. Performs depth-first rendering of portals, rendering
     the portal silhouettes to the stencil buffer on the way down, and the portal geometry
     and content on the way up.
     */
    void render(std::vector<tree<std::shared_ptr<VROPortal>>> &treeNodes,
                std::shared_ptr<VROPortal> outgoingTopPortal, bool renderBackgrounds,
                std::shared_ptr<VRORenderTarget> &target,
                const VRORenderContext &context,
                std::shared_ptr<VRODriver> &driver);
};

#endif /* VROPortalTreeRenderPass_h */
