//
//  VROScene.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/19/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//
#include <algorithm>
#include "VROScene.h"
#include "VRORenderContext.h"
#include "VRONode.h"
#include "VROPortal.h"
#include "VROPortalFrame.h"
#include "VROGeometry.h"
#include "VROInputControllerBase.h"
#include "VROLight.h"
#include "VROHitTestResult.h"
#include "VROMaterial.h"
#include "VROLog.h"
#include "VROAudioPlayer.h"
#include "VROSurface.h"
#include "VROMaterial.h"
#include "VROOpenGL.h" // For logging pglpush only
#include <stack>
#include <algorithm>

VROScene::VROScene() : VROThreadRestricted(VROThreadName::Renderer) {
    _rootNode = std::make_shared<VROPortal>();
    _rootNode->setName("Root");
    _activePortal = _rootNode;
    _activePortal->setPassable(true);
    
    _silhouetteMaterial = std::make_shared<VROMaterial>();
    _silhouetteMaterial->setWritesToDepthBuffer(false);
    _silhouetteMaterial->setReadsFromDepthBuffer(false);
    _silhouetteMaterial->setCullMode(VROCullMode::None);
    _silhouetteMaterial->addShaderModifier(VROPortalFrame::getAlphaDiscardModifier());
    
    ALLOCATION_TRACKER_ADD(Scenes, 1);
}

VROScene::~VROScene() {
    ALLOCATION_TRACKER_SUB(Scenes, 1);
}

#pragma mark - Rendering

void VROScene::render(const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    passert_thread();
    
    driver->enableColorBuffer();
    driver->clearDepthAndColor();
    driver->clearStencil(0);
    
    std::vector<tree<std::shared_ptr<VROPortal>>> treeNodes;
    treeNodes.push_back(_portals);
    render(treeNodes, context, driver);
}

// The key to this algorithm is we render depth-first. That is, we funnel down
// the tree, rendering portal silhouettes to the stencil buffer; then we unwind
// back up the tree, rendering the portal content. Only *then* do we move
// adjacently to the next sibling portal. Because we erase the stencil (via DECR
// commands) as we unwind the tree, each time we move to a sibling portal, all
// traces of the prior sibling should be gone. This ensures siblings don't bleed
// into each other (e.g. that an over-size object from one portal doesn't appear
// in any of its siblings).
void VROScene::render(std::vector<tree<std::shared_ptr<VROPortal>>> &treeNodes, const VRORenderContext &context,
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
            _silhouetteMaterial->bindShader(driver);
            _silhouetteMaterial->bindProperties(driver);
            
            if (portalFrame->isTwoSided()) {
                driver->disablePortalStencilWriting(portalFrame->getInactiveFace(isExit));
            }
            driver->disableColorBuffer();
            driver->enablePortalStencilWriting(portalFrame->getActiveFace(isExit));
            
            // Only render the portal silhouette over the area covered
            // by the parent portal. Clip the rest (we don't want a portal
            // within a portal to bleed outside of its parent).
            driver->setStencilPassBits(portalFrame->getActiveFace(isExit), portal->getRecursionLevel() - 1, false);
            portal->renderPortalSilhouette(_silhouetteMaterial, VROSilhouetteMode::Textured, context, driver);
            pglpop();
        }
        
        // Recurse down to children. This way we continue rendering portal
        // silhouettes (of children, not siblings) before moving on to rendering
        // actual content.
        render(treeNode.children, context, driver);
        
        // Now we're unwinding from recursion, prepare for scene rendering.
        pglpush("Contents");
        driver->enableColorBuffer();
        driver->disablePortalStencilWriting(VROFace::FrontAndBack);
        
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
        driver->setStencilPassBits(VROFace::FrontAndBack, portal->getRecursionLevel(), true);
        portal->renderBackground(context, driver);
        portal->renderContents(context, driver);
        pglpop();
        
        if (portalFrame) {
            // Remove the stencil for this portal (decrement its number). Ensures
            // side-by-side portals (portals with same recursion level) work correctly;
            // otherwise objects in one portal can "bleed" into the other portal.
            pglpush("(-) Stencil");
            _silhouetteMaterial->bindShader(driver);
            _silhouetteMaterial->bindProperties(driver);
            
            driver->disableColorBuffer();
            driver->enablePortalStencilRemoval(portalFrame->getActiveFace(isExit));
            driver->setStencilPassBits(portalFrame->getActiveFace(isExit), portal->getRecursionLevel(), true);
            portal->renderPortalSilhouette(_silhouetteMaterial, VROSilhouetteMode::Textured, context, driver);
            pglpop();
            
            // Finally, render the portal frame to the color and depth buffers.
            pglpush("Portal Frame");
            driver->enableColorBuffer();
            driver->disablePortalStencilWriting(VROFace::FrontAndBack);
            driver->setStencilPassBits(portalFrame->getActiveFace(isExit), portal->getRecursionLevel() - 1, true);
            portal->renderPortal(context, driver);
            pglpop();
        }
        
        ++i;
        pglpop();
    }
}

void VROScene::computeTransforms(const VRORenderContext &context) {
    _rootNode->computeTransforms({}, {});
}

void VROScene::updateVisibility(const VRORenderContext &context) {
    _rootNode->updateVisibility(context);
}

void VROScene::applyConstraints(const VRORenderContext &context) {
    _rootNode->applyConstraints(context, {}, false);
}

void VROScene::updateSortKeys(const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    passert_thread();
    
    if (kDebugSortOrder) {
        pinfo("Updating sort keys");
        VRONode::resetDebugSortIndex();
    }

    VRORenderParameters renderParams;
    _rootNode->collectLights(&renderParams.lights);
    _rootNode->updateSortKeys(0, renderParams, context, driver);
    
    createPortalTree(context);
    _portals.walkTree([] (std::shared_ptr<VROPortal> portal) {
        portal->sortNodesBySortKeys();
    });
    
    _distanceOfFurthestObjectFromCamera = renderParams.furthestDistanceFromCamera;
}

#pragma mark - Portals

std::shared_ptr<VROPortal> VROScene::getRootNode() {
    return _rootNode;
}

void VROScene::setActivePortal(const std::shared_ptr<VROPortal> portal) {
    passert (hasNode(std::dynamic_pointer_cast<VRONode>(portal)));
    _activePortal = portal;
}

void VROScene::createPortalTree(const VRORenderContext &context) {
    _portals.children.clear();
    _portals.value.reset();
    _activePortal->traversePortals(context.getFrame(), 0, nullptr, &_portals);
    
    // Sort each recursion level by distance from camera, so that we render
    // sibling portals (portals on same recursion level) front to back
    sortSiblingPortals(_portals, context);
}

void VROScene::sortSiblingPortals(tree<std::shared_ptr<VROPortal>> &node, const VRORenderContext &context) {
    std::vector<tree<std::shared_ptr<VROPortal>>> &portals = node.children;
    std::sort(portals.begin(), portals.end(), [context](tree<std::shared_ptr<VROPortal>> &a, tree<std::shared_ptr<VROPortal>> &b) {
        passert (a.value->getRecursionLevel() == b.value->getRecursionLevel());
        return a.value->getComputedPosition().distance(context.getCamera().getPosition()) <
               b.value->getComputedPosition().distance(context.getCamera().getPosition());
    });
    
    for (tree<std::shared_ptr<VROPortal>> &child : portals) {
        sortSiblingPortals(child, context);
    }
}

bool VROScene::hasNode(std::shared_ptr<VRONode> node) const {
    return hasNode_helper(_rootNode, node);
}

bool VROScene::hasNode_helper(const std::shared_ptr<VRONode> &candidate, const std::shared_ptr<VRONode> &node) const {
    if (candidate == node) {
        return true;
    }
    else {
        for (std::shared_ptr<VRONode> &child : candidate->getChildNodes()) {
            if (hasNode_helper(child, node)) {
                return true;
            }
        }
        return false;
    }
}

const tree<std::shared_ptr<VROPortal>> VROScene::getPortalTree() const {
    return _portals;
}

#pragma mark - Input Controllers

void VROScene::detachInputController(std::shared_ptr<VROInputControllerBase> controller){
    passert_thread();
    if (!_controllerPresenter){
        return;
    }

    std::shared_ptr<VRONode> node = _controllerPresenter->getRootNode();
    node->removeFromParentNode();
    
    controller->detachScene();
    _controllerPresenter = nullptr;
}

void VROScene::attachInputController(std::shared_ptr<VROInputControllerBase> controller) {
    passert_thread();

    std::shared_ptr<VROInputPresenter> presenter = controller->getPresenter();
    if (_controllerPresenter == presenter) {
        return;
    }

    std::shared_ptr<VRONode> node = presenter->getRootNode();
    _rootNode->addChildNode(node);
    _controllerPresenter = presenter;

    controller->attachScene(shared_from_this());
}

std::shared_ptr<VROInputPresenter> VROScene::getControllerPresenter(){
    return _controllerPresenter;
}

std::vector<std::shared_ptr<VROGeometry>> VROScene::getBackgrounds() const {
    std::vector<std::shared_ptr<VROGeometry>> backgrounds;
    getBackgrounds(_rootNode, backgrounds);

    return backgrounds;
}

void VROScene::getBackgrounds(std::shared_ptr<VRONode> node, std::vector<std::shared_ptr<VROGeometry>> &backgrounds) const {
    if (node->getType() == VRONodeType::Portal) {
        std::shared_ptr<VROPortal> portal = std::dynamic_pointer_cast<VROPortal>(node);
        if (portal->getBackground() != nullptr) {
            backgrounds.push_back(portal->getBackground());
        }
    }
    
    for (std::shared_ptr<VRONode> &child : node->getChildNodes()) {
        getBackgrounds(child, backgrounds);
    }
}

void VROScene::drawBoundingBoxCorners(std::shared_ptr<VRONode> node,
                                      const VRORenderContext &context,
                                      std::shared_ptr<VRODriver> &driver) {
    std::shared_ptr<VROPencil> pencil = std::make_shared<VROPencil>();
    VROVector3f minPoint = VROVector3f(node->getBoundingBox().getMinX(), node->getBoundingBox().getMinY(), node->getBoundingBox().getMinZ());
    VROVector3f maxPoint = VROVector3f(node->getBoundingBox().getMaxX(), node->getBoundingBox().getMaxY(), node->getBoundingBox().getMaxZ());
    pencil->draw(minPoint, maxPoint);
    pencil->render(context, driver);
}

