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
#include "VROCamera.h"
#include "VROScene.h"
#include "VROLog.h"

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
        VROLineSegment segment(context.getPreviousCamera().getPosition(), context.getCamera().getPosition());
        
        const tree<std::shared_ptr<VROPortal>> &portalTree = scene->getPortalTree();
        std::shared_ptr<VROPortal> portal = findPortalTraversal(segment, portalTree);
        if (portal) {
            scene->setActivePortal(portal);
        }
        
        // VIRO-1400 TODO: remove this log statement
        pinfo("Diff %f, %f, %f", diff.x, diff.y, diff.z);
    }
}

void VROPortalTraversalListener::onFrameDidRender(const VRORenderContext &context) {
    
}

std::shared_ptr<VROPortal> VROPortalTraversalListener::findPortalTraversal(const VROLineSegment &segment,
                                                                           const tree<std::shared_ptr<VROPortal>> &portalTree) {
    const std::shared_ptr<VROPortal> &portal = portalTree.value;
    if (portal->intersectsLineSegment(segment)) {
        return portal;
    }
    else {
        // Recurse, returning the first child with a valid intersection
        for (tree<std::shared_ptr<VROPortal>> child : portalTree.children) {
            const std::shared_ptr<VROPortal> result = findPortalTraversal(segment, child);
            if (result) {
                return result;
            }
        }
        
        // Return null if no portal traversed
        return nullptr;
    }
}
