//
//  VROPortalTraversalListener.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/3/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPortalTraversalListener.h"
#include "VRORenderContext.h"
#include "VROVector3f.h"
#include "VROLineSegment.h"
#include "VROPortal.h"
#include "VROPortalFrame.h"
#include "VROCamera.h"
#include "VROScene.h"
#include "VRORenderer.h"
#include "VROLog.h"

static const float kDistanceToRestoreTwoSidedPortal = 1.0;

VROPortalTraversalListener::VROPortalTraversalListener(std::shared_ptr<VROScene> scene) :
    _scene(scene) {
    
}

VROPortalTraversalListener::~VROPortalTraversalListener() {
    
}

void VROPortalTraversalListener::onFrameWillRender(const VRORenderContext &context) {
    std::shared_ptr<VROScene> scene = _scene.lock();
    if (!scene) {
        return;
    }
    
    VROVector3f diff = context.getCamera().getPosition() - context.getPreviousCamera().getPosition();
    if (diff.magnitude() > 0) {
        // Take the camera diff and shift it forward by the NCP plus some epsilon
        // margin. We need to transition *before* the portal gets clipped, otherwise
        // we get a flicker.
        VROLineSegment segment(context.getPreviousCamera().getPosition(), context.getCamera().getPosition());
        segment = segment.translate(context.getCamera().getForward().scale(kZNear * 2.0));
        
        const tree<std::shared_ptr<VROPortal>> &portalTree = scene->getPortalTree();
        std::shared_ptr<VROPortal> portal = findPortalTraversal(segment, portalTree);
        if (portal) {
            portal->getActivePortalFrame()->setTwoSided(true);
            scene->setActivePortal(portal);
        }
        
        restorePortalFaces(context.getCamera().getPosition(), portalTree);
    }
}

void VROPortalTraversalListener::onFrameDidRender(const VRORenderContext &context) {
    
}

std::shared_ptr<VROPortal> VROPortalTraversalListener::findPortalTraversal(const VROLineSegment &segment,
                                                                           const tree<std::shared_ptr<VROPortal>> &portalTree) {
    // Return the first child with a valid intersection
    for (tree<std::shared_ptr<VROPortal>> child : portalTree.children) {
        if (child.value) {
            if (child.value->isPassable() && child.value->intersectsLineSegment(segment)) {
                return child.value;
            }
        }
    }
    
    // Return null if no portal traversed
    return nullptr;
}

void VROPortalTraversalListener::restorePortalFaces(const VROVector3f &cameraPosition,
                                                    const tree<std::shared_ptr<VROPortal>> &portalTree) {
    const std::shared_ptr<VROPortal> &portal = portalTree.value;
    if (portal->isPassable() &&
        portal->getActivePortalFrame() &&
        portal->getActivePortalFrame()->isTwoSided() &&
        portal->getActivePortalFrame()->getComputedPosition().distance(cameraPosition) > kDistanceToRestoreTwoSidedPortal) {
        portal->getActivePortalFrame()->setTwoSided(false);
    }
    for (tree<std::shared_ptr<VROPortal>> child : portalTree.children) {
        restorePortalFaces(cameraPosition, child);
    }
}
