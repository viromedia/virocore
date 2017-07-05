//
//  VROARComponentManager.cpp
//  ViroKit
//
//  Created by Andy Chu on 6/16/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARComponentManager.h"
#include "VROLog.h"

void VROARComponentManager::addARPlane(std::shared_ptr<VROARPlane> plane) {
    handleDetachedPlane(plane);
}

void VROARComponentManager::removeARPlane(std::shared_ptr<VROARPlane> plane) {
    std::shared_ptr<VROARAnchor> anchor = plane->getAnchor();
    // if plane had an anchor, then the anchor is now detached, otherwise remove the plane
    // from the detachedPlanes vector.
    if (anchor) {
        handleDetachedAnchor(anchor);
    } else {
        _detachedPlanes.erase(std::remove_if(_detachedPlanes.begin(), _detachedPlanes.end(),
                                             [plane](std::shared_ptr<VROARPlane> candidate) {
                                                 return candidate == plane;
                                             }), _detachedPlanes.end());
    }
}

void VROARComponentManager::updateARPlane(std::shared_ptr<VROARPlane> plane) {
    std::shared_ptr<VROARAnchor> anchor = plane->getAnchor();
    // if the plane doesn't match the anchor anymore, then detach plane and anchor
    if (anchor) {
        if (plane->hasRequirementsFulfilled(anchor)) {
            return;
        }
        plane->onARAnchorRemoved();
        handleDetachedAnchor(anchor);
        addAnchorToSession(anchor);
    }
    handleDetachedPlane(plane);
}

std::shared_ptr<VROARAnchor> VROARComponentManager::findDetachedAnchor(std::shared_ptr<VROARPlane> plane) {
    std::vector<std::shared_ptr<VROARAnchor>>::iterator it;
    for (it = _detachedAnchors.begin(); it < _detachedAnchors.end(); it++) {
        if (plane->hasRequirementsFulfilled(*it)) {
            return *it;
        }
    }
    return nullptr;
}

std::shared_ptr<VROARPlane> VROARComponentManager::findDetachedPlane(std::shared_ptr<VROARAnchor> anchor) {
    std::vector<std::shared_ptr<VROARPlane>>::iterator it;
    for (it = _detachedPlanes.begin(); it < _detachedPlanes.end(); it++) {
        std::shared_ptr<VROARPlane> candidate = *it;
        if (candidate->hasRequirementsFulfilled(anchor)) {
            return candidate;
        }
    }
    return nullptr;
}

void VROARComponentManager::handleDetachedPlane(std::shared_ptr<VROARPlane> plane) {
    std::shared_ptr<VROARAnchor> anchor = findDetachedAnchor(plane);
    if (anchor) {
        // remove the anchor from the _detachedAnchor vector
        _detachedAnchors.erase(std::remove_if(_detachedAnchors.begin(), _detachedAnchors.end(),
                                              [anchor](std::shared_ptr<VROARAnchor> candidate) {
                                                  return candidate == anchor;
                                              }), _detachedAnchors.end());
        plane->setAnchor(anchor);
        anchor->setARNode(plane);
        // call "addAnchorToSession" because we've attached this anchor to a node, so the session should render it.
        addAnchorToSession(anchor);
        plane->onARAnchorAttached();
    } else {
        _detachedPlanes.push_back(plane);
        // since this plane was detached, set its anchor to null.
        plane->setAnchor(nullptr);
    }
}

void VROARComponentManager::handleDetachedAnchor(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARPlane> plane = findDetachedPlane(anchor);
    if (plane) {
        // remove the plane from the _detachedPlanes vector
        _detachedPlanes.erase(std::remove_if(_detachedPlanes.begin(), _detachedPlanes.end(),
                                             [plane](std::shared_ptr<VROARPlane> candidate) {
                                                 return candidate == plane;
                                             }), _detachedPlanes.end());
        plane->setAnchor(anchor);
        anchor->setARNode(plane);
        plane->onARAnchorAttached();
        // don't call "addAnchorToSession" because currently the Session does that on the anchorWasDetected callback
    } else {
        _detachedAnchors.push_back(anchor);
        // since this anchor is "detached" we should set its ARNode to null.
        anchor->setARNode(nullptr);
    }
}

void VROARComponentManager::addAnchorToSession(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARSession> session = _session.lock();
    std::shared_ptr<VROARNode> arNode = anchor->getARNode();
    if (session && arNode) {
        session->addAnchorNode(arNode);
    }
}

// VROARSessionDelegate functions
std::shared_ptr<VROARNode> VROARComponentManager::anchorWasDetected(std::shared_ptr<VROARAnchor> anchor) {
    handleDetachedAnchor(anchor);
    std::shared_ptr<VROARNode> arNode = anchor->getARNode();
    return arNode ? arNode : std::make_shared<VROARNode>();
}

void VROARComponentManager::anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor) {
    
}

void VROARComponentManager::anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARPlane> arPlane = std::dynamic_pointer_cast<VROARPlane>(anchor->getARNode());
    if (arPlane) {
        if (arPlane->hasRequirementsFulfilled(anchor)) {
            arPlane->onARAnchorUpdated();
            return;
        }
        arPlane->onARAnchorRemoved();
        handleDetachedPlane(arPlane);
    }
    handleDetachedAnchor(anchor);
    addAnchorToSession(anchor);
}

void VROARComponentManager::anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARPlane> plane = std::dynamic_pointer_cast<VROARPlane>(anchor->getARNode());
    // if anchor was attached to a plane, then "handle" the detached plane.
    if (plane) {
        plane->onARAnchorRemoved();
        handleDetachedPlane(plane);
    } else {
        // remove anchor from list.
        _detachedAnchors.erase(std::remove_if(_detachedAnchors.begin(), _detachedAnchors.end(),
                                              [anchor](std::shared_ptr<VROARAnchor> candidate) {
                                                  return candidate == anchor;
                                              }), _detachedAnchors.end());
    }
}
