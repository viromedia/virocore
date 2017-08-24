//
//  VROPortalTreeRenderPass.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPortalTreeRenderPass.h"
#include "VROLog.h"
#include "VROMaterial.h"
#include "VRORenderTarget.h"
#include "VRODriver.h"
#include "VROPortal.h"
#include "VROScene.h"
#include "VRORenderContext.h"
#include "VROLight.h"
#include "VROPortalFrame.h"
#include "VROShadowMapRenderPass.h" // For drawing light frustra

VROPortalTreeRenderPass::VROPortalTreeRenderPass() {
    _silhouetteMaterial = std::make_shared<VROMaterial>();
    _silhouetteMaterial->setWritesToDepthBuffer(false);
    _silhouetteMaterial->setReadsFromDepthBuffer(false);
    _silhouetteMaterial->setCullMode(VROCullMode::None);
    _silhouetteMaterial->addShaderModifier(VROPortalFrame::getAlphaDiscardModifier());
}

VROPortalTreeRenderPass::~VROPortalTreeRenderPass() {
    
}

VRORenderPassInputOutput VROPortalTreeRenderPass::render(std::shared_ptr<VROScene> scene, VRORenderPassInputOutput &inputs,
                                                         VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    
    std::shared_ptr<VRORenderTarget> target = inputs[kRenderTargetSingleOutput];
    passert (target);
    target->bind();
    
    driver->setColorWritingEnabled(true);
    target->clearDepthAndColor();
    target->clearStencil(0);
    
    if (kDebugShadowMaps) {
        drawLightFrustra(scene, context, driver);
    }
    
    std::vector<tree<std::shared_ptr<VROPortal>>> treeNodes;
    treeNodes.push_back(scene->getPortalTree());
    render(treeNodes, target, *context, driver);
    
    VRORenderPassInputOutput output;
    output[kRenderTargetSingleOutput] = target;
    
    return output;
}

// The key to this algorithm is we render depth-first. That is, we funnel down
// the tree, rendering portal silhouettes to the stencil buffer; then we unwind
// back up the tree, rendering the portal content. Only *then* do we move
// adjacently to the next sibling portal. Because we erase the stencil (via DECR
// commands) as we unwind the tree, each time we move to a sibling portal, all
// traces of the prior sibling should be gone. This ensures siblings don't bleed
// into each other (e.g. that an over-size object from one portal doesn't appear
// in any of its siblings).
void VROPortalTreeRenderPass::render(std::vector<tree<std::shared_ptr<VROPortal>>> &treeNodes,
                                     std::shared_ptr<VRORenderTarget> &target,
                                     const VRORenderContext &context,
                                     std::shared_ptr<VRODriver> &driver) {
    
    // Iterate through each sibling at this recursion level. The siblings should be ordered
    // front to back. Ensures that the transparent sheens (or 'windows') of each
    // portal are written to the depth buffer *before* the portals behind them are
    // rendered. Otherwise blending would cause portals on the same recursion level
    // to appear through one another.
    int i = 0;
    for (tree<std::shared_ptr<VROPortal>> &treeNode : treeNodes) {
        std::shared_ptr<VROPortal> &portal = treeNode.value;
        
        const std::shared_ptr<VROPortalFrame> &portalFrame = portal->getActivePortalFrame();
        bool isExit = portal->isRenderingExitFrame();
        
        pglpush("Recursion Level %d, Portal %d [%s]", portal->getRecursionLevel(), i, portal->getName().c_str());
        
        // Render the portal first to the stencil buffer only. We have to render
        // with textures, because the texture's alpha portions determine where
        // we discard fragments (we only write the transparent sections to the stencil
        // buffer).
        if (portalFrame) {
            pglpush("(+) Stencil");
            _silhouetteMaterial->bindShader(0, {}, driver);
            _silhouetteMaterial->bindProperties(driver);
            
            if (portalFrame->isTwoSided()) {
                target->disablePortalStencilWriting(portalFrame->getInactiveFace(isExit));
            }
            driver->setColorWritingEnabled(false);
            target->enablePortalStencilWriting(portalFrame->getActiveFace(isExit));
            
            // Only render the portal silhouette over the area covered
            // by the parent portal. Clip the rest (we don't want a portal
            // within a portal to bleed outside of its parent).
            target->setStencilPassBits(portalFrame->getActiveFace(isExit), portal->getRecursionLevel() - 1, false);
            portal->renderPortalSilhouette(_silhouetteMaterial, VROSilhouetteMode::Textured, VROSilhouetteFilter::None,
                                           context, driver);
            
            driver->unbindShader();
            pglpop();
        }
        
        // Recurse down to children. This way we continue rendering portal
        // silhouettes (of children, not siblings) before moving on to rendering
        // actual content.
        render(treeNode.children, target, context, driver);
        
        // Now we're unwinding from recursion, prepare for scene rendering.
        pglpush("Contents");
        driver->setColorWritingEnabled(true);
        target->disablePortalStencilWriting(VROFace::FrontAndBack);
        
        // Draw wherever the stencil buffer value is greater than or equal
        // to the recursion level of this portal. This has two effects:
        //
        // 1. It means we draw over areas belonging to this recursion level
        //    *and* over areas belonging to deeper recursion levels. This
        //    enables an object at level 1 to occlude a portal into level 2,
        //    for example.
        // 2. It ensures that no objects at this level are drawn into any upper
        //    levels. An object at level 2 will not be drawn into an area
        //    belonging to level 1.
        target->setStencilPassBits(VROFace::FrontAndBack, portal->getRecursionLevel(), true);
        portal->renderBackground(context, driver);
        portal->renderContents(context, driver);
        driver->unbindShader();
        pglpop();
        
        if (portalFrame) {
            // Remove the stencil for this portal (decrement its number). Ensures
            // side-by-side portals (portals with same recursion level) work correctly;
            // otherwise objects in one portal can "bleed" into the other portal.
            pglpush("(-) Stencil");
            _silhouetteMaterial->bindShader(0, {}, driver);
            _silhouetteMaterial->bindProperties(driver);
            
            driver->setColorWritingEnabled(false);
            target->enablePortalStencilRemoval(portalFrame->getActiveFace(isExit));
            target->setStencilPassBits(portalFrame->getActiveFace(isExit), portal->getRecursionLevel(), true);
            portal->renderPortalSilhouette(_silhouetteMaterial, VROSilhouetteMode::Textured, VROSilhouetteFilter::None,
                                           context, driver);
            driver->unbindShader();
            pglpop();
            
            // Finally, render the portal frame to the color and depth buffers. Note
            // we need to render the transparent section of the portal to the depth
            // buffer in order to prevent inner portal backgrounds from covering it up.
            // (so culling *must* be disabled on the portal frame, in case we're looking
            // at the transparent section from behind).
            pglpush("Portal Frame");
            driver->setColorWritingEnabled(true);
            target->disablePortalStencilWriting(VROFace::FrontAndBack);
            target->setStencilPassBits(portalFrame->getActiveFace(isExit), portal->getRecursionLevel() - 1, true);
            portal->renderPortal(context, driver);
            driver->unbindShader();
            pglpop();
        }
        
        ++i;
        pglpop();
    }
}

void VROPortalTreeRenderPass::drawLightFrustra(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                               std::shared_ptr<VRODriver> &driver) {
    const std::vector<std::shared_ptr<VROLight>> &lights = scene->getLights();
    for (const std::shared_ptr<VROLight> &light : lights) {
        if (!light->getCastsShadow()) {
            continue;
        }
        light->drawLightFrustum(context->getPencil());
    }
    context->getPencil()->render(*context, driver);
    context->getPencil()->clear();
}
