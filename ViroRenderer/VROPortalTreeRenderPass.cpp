//
//  VROPortalTreeRenderPass.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROPortalTreeRenderPass.h"
#include "VROLog.h"
#include "VROMaterial.h"
#include "VRORenderTarget.h"
#include "VRODriver.h"
#include "VROPortal.h"
#include "VROScene.h"
#include "VRORenderContext.h"
#include "VROPencil.h"
#include "VROLight.h"
#include "VROPortalFrame.h"
#include "VROOpenGL.h" // For pglpush and pop
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

void VROPortalTreeRenderPass::render(std::shared_ptr<VROScene> scene,
                                     std::shared_ptr<VROScene> outgoingScene,
                                     VRORenderPassInputOutput &inputs,
                                     VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {

    std::shared_ptr<VRORenderTarget> target = inputs.outputTarget;
    passert (target);
    
    driver->bindRenderTarget(target, VRORenderTargetUnbindOp::Invalidate);
    driver->setRenderTargetColorWritingMask(VROColorMaskAll);

    // Get the top portal for the outgoing tree if we have an outgoing scene; this
    // way we can render the background of the outgoing scene with the background
    // of the regular scene, preventing blending artifacts during transitions
    std::vector<tree<std::shared_ptr<VROPortal>>> outgoingTreeNodes;
    std::shared_ptr<VROPortal> outgoingTopPortal;
    if (outgoingScene) {
        outgoingTreeNodes.push_back(outgoingScene->getPortalTree());
        if (!outgoingTreeNodes.empty()) {
            outgoingTopPortal = outgoingTreeNodes.front().value;
        }
    }

    // Render the regular scene; if an outgoing scene is present this will render
    // its top-level background as well
    std::vector<tree<std::shared_ptr<VROPortal>>> treeNodes;
    treeNodes.push_back(scene->getPortalTree());
    render(treeNodes, outgoingTopPortal, true, target, *context, driver);

    // Render the outgoing scene (if available). The outgoing scene is rendered
    // without backgrounds here
    if (outgoingScene) {
        render(outgoingTreeNodes, nullptr, false, target, *context, driver);
    }

    // Render the pencil
    context->getPencil()->render(*context, driver);
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
                                     std::shared_ptr<VROPortal> outgoingTopPortal, bool renderBackgrounds,
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
            _silhouetteMaterial->bindShader(0, {}, context, driver);
            _silhouetteMaterial->bindProperties(driver);
            
            if (portalFrame->isTwoSided()) {
                target->disablePortalStencilWriting(portalFrame->getInactiveFace(isExit));
            }
            driver->setRenderTargetColorWritingMask(VROColorMaskNone);
            target->enablePortalStencilWriting(portalFrame->getActiveFace(isExit));
            
            // Only render the portal silhouette over the area covered
            // by the parent portal. Clip the rest (we don't want a portal
            // within a portal to bleed outside of its parent).
            target->setPortalStencilPassFunction(portalFrame->getActiveFace(isExit), VROStencilFunc::Equal,
                                                 portal->getRecursionLevel() - 1);
            portal->renderPortalSilhouette(_silhouetteMaterial, VROSilhouetteMode::Textured, nullptr,
                                           context, driver);
            
            driver->unbindShader();
            pglpop();
        }
        
        // Recurse down to children. This way we continue rendering portal
        // silhouettes (of children, not siblings) before moving on to rendering
        // actual content.
        render(treeNode.children, nullptr, true, target, context, driver);
        
        // Now we're unwinding from recursion, prepare for scene rendering.
        pglpush("Contents");
        driver->setRenderTargetColorWritingMask(VROColorMaskAll);
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
        target->setPortalStencilPassFunction(VROFace::FrontAndBack, VROStencilFunc::LessOrEqual,
                                             portal->getRecursionLevel());
        if (renderBackgrounds) {
            if (outgoingTopPortal != nullptr && i == 0) {
                outgoingTopPortal->renderBackground(context, driver);
            }
            portal->renderBackground(context, driver);
        }
        portal->renderContents(context, driver);
        driver->unbindShader();
        pglpop();
        
        if (portalFrame) {
            // Remove the stencil for this portal (decrement its number). Ensures
            // side-by-side portals (portals with same recursion level) work correctly;
            // otherwise objects in one portal can "bleed" into the other portal.
            pglpush("(-) Stencil");
            _silhouetteMaterial->bindShader(0, {}, context, driver);
            _silhouetteMaterial->bindProperties(driver);
            
            driver->setRenderTargetColorWritingMask(VROColorMaskNone);
            target->enablePortalStencilRemoval(portalFrame->getActiveFace(isExit));
            target->setPortalStencilPassFunction(portalFrame->getActiveFace(isExit), VROStencilFunc::LessOrEqual,
                                                 portal->getRecursionLevel());
            portal->renderPortalSilhouette(_silhouetteMaterial, VROSilhouetteMode::Textured, nullptr,
                                           context, driver);
            driver->unbindShader();
            pglpop();
            
            // Finally, render the portal frame to the color and depth buffers. Note
            // we need to render the transparent section of the portal to the depth
            // buffer in order to prevent inner portal backgrounds from covering it up.
            // (so culling *must* be disabled on the portal frame, in case we're looking
            // at the transparent section from behind).
            pglpush("Portal Frame");
            driver->setRenderTargetColorWritingMask(VROColorMaskAll);
            target->disablePortalStencilWriting(VROFace::FrontAndBack);
            target->setPortalStencilPassFunction(portalFrame->getActiveFace(isExit), VROStencilFunc::LessOrEqual,
                                                 portal->getRecursionLevel() - 1);
            portal->renderPortal(context, driver);
            driver->unbindShader();
            pglpop();
        }
        
        ++i;
        pglpop();
    }
}
