//
//  VROPortalTraversalListener.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/3/17.
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
        // Take the camera diff and shift it forward toward the NCP (e.g. so that
        // it's tip touches the NCP). Scale it bit by an epsilon multiplier so that
        // its tip actually breaches the NCP. Then then check if this segment
        // intersects any portal.
        
        // We need to transition *before* the portal gets clipped, otherwise
        // we get a flicker. If flickers occur, try increasing the epsilon multiplier.
        float epsilonMultiplier = 4.0;
        VROLineSegment segment(context.getPreviousCamera().getPosition(), context.getCamera().getPosition());
        segment = segment.translate(context.getCamera().getForward().scale(kZNear * epsilonMultiplier));
        
        const tree<std::shared_ptr<VROPortal>> &portalTree = scene->getPortalTree();
        std::shared_ptr<VROPortal> portal = findPortalTraversal(segment, portalTree);
        if (portal) {
            portal->getActivePortalFrame()->setTwoSided(true);
            
            std::shared_ptr<VROPortal> currentActivePortal = scene->getActivePortal();
            if (currentActivePortal) {
                if (currentActivePortal->getPortalDelegate() != nullptr) {
                    currentActivePortal->getPortalDelegate()->onPortalExit();
                }
            }
            if (portal->getPortalDelegate() != nullptr) {
                portal->getPortalDelegate()->onPortalEnter();
            }
            
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
    if (portal && portal->isPassable() &&
        portal->getActivePortalFrame() &&
        portal->getActivePortalFrame()->isTwoSided() &&
            portal->getActivePortalFrame()->getWorldPosition().distance(cameraPosition) > kDistanceToRestoreTwoSidedPortal) {
        portal->getActivePortalFrame()->setTwoSided(false);
    }
    for (tree<std::shared_ptr<VROPortal>> child : portalTree.children) {
        restorePortalFaces(cameraPosition, child);
    }
}
