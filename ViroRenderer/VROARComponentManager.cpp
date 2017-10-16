//
//  VROARComponentManager.cpp
//  ViroKit
//
//  Created by Andy Chu on 6/16/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARComponentManager.h"
#include "VROLog.h"

void VROARComponentManager::addARPlane(std::shared_ptr<VROARPlaneNode> plane) {
    processDetachedPlane(plane);
}

void VROARComponentManager::updateARPlane(std::shared_ptr<VROARPlaneNode> plane) {
    std::shared_ptr<VROARAnchor> anchor = plane->getAnchor();

    // if the plane has an anchor, then see if the anchor still fulfills the plane's requirements
    if (anchor) {
        if (plane->hasRequirementsFulfilled(anchor) || plane->getId() == anchor->getId()) {
            return;
        }
        // notify the plane of detachment, detach anchor & plane, notify anchor updated.
        plane->onARAnchorRemoved();
        anchor->setARNode(nullptr);
        notifyAnchorWasUpdated(anchor);

        processDetachedAnchor(anchor);
    }
    processDetachedPlane(plane);
}

void VROARComponentManager::removeARPlane(std::shared_ptr<VROARPlaneNode> plane) {
    std::shared_ptr<VROARAnchor> anchor = plane->getAnchor();
    if (anchor) {
        // detach the plane from anchor and notify anchor updated
        anchor->setARNode(nullptr);
        notifyAnchorWasUpdated(anchor);

        processDetachedAnchor(anchor);
    } else {
        removeFromDetachedList(plane);
    }
}

void VROARComponentManager::setDelegate(std::shared_ptr<VROARComponentManagerDelegate> delegate) {
    _delegate = delegate;
    // When attaching a new delegate, notify delegate of all "detached" anchors.
    if (delegate) {
        std::vector<std::shared_ptr<VROARAnchor>>::iterator it;
        for (it = _detachedAnchors.begin(); it < _detachedAnchors.end(); it++) {
            std::shared_ptr<VROARAnchor> anchor = *it;
            delegate->anchorWasDetected(anchor);
        }
    }
}

void VROARComponentManager::clearAllPlanes(std::vector<std::shared_ptr<VROARPlaneNode>> planes) {
    std::vector<std::shared_ptr<VROARPlaneNode>>::iterator it;
    for (it = planes.begin(); it < planes.end(); it++) {
        std::shared_ptr<VROARPlaneNode> plane = *it;
        
        // if the plane had an anchor, then add it back to _detachedAnchors
        std::shared_ptr<VROARAnchor> anchor = plane->getAnchor();
        if (anchor) {
            anchor->setARNode(nullptr);
            _detachedAnchors.push_back(anchor);
        }
    }
}

#pragma mark - VROARSessionDelegate Implementation

void VROARComponentManager::anchorWasDetected(std::shared_ptr<VROARAnchor> anchor) {
    // add anchor to map
    _nativeAnchorMap[anchor->getId()] = anchor;

    notifyAnchorWasDetected(anchor);
    processDetachedAnchor(anchor);
}

void VROARComponentManager::anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor) {
    // no-op
}

void VROARComponentManager::anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor) {
    notifyAnchorWasUpdated(anchor);
    std::shared_ptr<VROARPlaneNode> arPlane = std::dynamic_pointer_cast<VROARPlaneNode>(anchor->getARNode());
    if (arPlane) {
        if (arPlane->hasRequirementsFulfilled(anchor)) {
            arPlane->onARAnchorUpdated();
            return;
        }
        // if the updated anchor doesn't fulfill the plane's requirement, notify that it was removed
        // before attempting to reattach it to another anchor.
        arPlane->onARAnchorRemoved();
        arPlane->setAnchor(nullptr);
        processDetachedPlane(arPlane);
    }
    processDetachedAnchor(anchor);
}

void VROARComponentManager::anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor) {
    notifyAnchorWasRemoved(anchor);
    std::shared_ptr<VROARPlaneNode> plane = std::dynamic_pointer_cast<VROARPlaneNode>(anchor->getARNode());
    // if anchor was attached to a plane, notify the plane that anchor was removed then "handle" the detached plane.
    if (plane) {
        plane->onARAnchorRemoved();
        plane->setAnchor(nullptr);
        processDetachedPlane(plane);
    } else {
        removeFromDetachedList(anchor);
    }

    // remove anchor from map
    auto it = _nativeAnchorMap.find(anchor->getId());
    if (it != _nativeAnchorMap.end()) {
        _nativeAnchorMap.erase(it);
    }

}

#pragma mark - Internal Functions

std::shared_ptr<VROARAnchor> VROARComponentManager::getAnchorFromId(std::string id) {
    auto it = _nativeAnchorMap.find(id);
    if (it != _nativeAnchorMap.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}


std::shared_ptr<VROARAnchor> VROARComponentManager::findDetachedAnchor(std::shared_ptr<VROARPlaneNode> plane) {
    std::vector<std::shared_ptr<VROARAnchor>>::iterator it;
    for (it = _detachedAnchors.begin(); it < _detachedAnchors.end(); it++) {
        if (plane->hasRequirementsFulfilled(*it)) {
            return *it;
        }
    }
    return nullptr;
}

void VROARComponentManager::processDetachedPlane(std::shared_ptr<VROARPlaneNode> plane) {
    std::string id = plane->getId();
    if (!id.empty()) {
        // first find if there's an anchor!
        std::shared_ptr<VROARAnchor> anchor = getAnchorFromId(id);
        if (anchor) {
            removeFromDetachedList(anchor);
            std::shared_ptr<VROARPlaneNode> oldPlane = std::dynamic_pointer_cast<VROARPlaneNode>(anchor->getARNode());
            if (oldPlane) {
                if (oldPlane->getId() == anchor->getId()) {
                    // do nothing if the oldPlane shared the same ID
                    return;
                }
                // detach the oldPlane and process it as detached plane if it doesn't have the same ID
                oldPlane->setAnchor(nullptr);
                oldPlane->onARAnchorRemoved();
                processDetachedPlane(oldPlane);
            }
            // attach the given plane to the anchor that shares its ID.
            attachNodeToAnchor(plane, anchor);
        }
    } else {
        std::shared_ptr<VROARAnchor> anchor = findDetachedAnchor(plane);
        if (anchor) {
            removeFromDetachedList(anchor);
            attachNodeToAnchor(plane, anchor);
        } else {
            _detachedPlanes.push_back(plane);
        }
    }
}

void VROARComponentManager::processDetachedAnchor(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARPlaneNode> plane = findDetachedPlane(anchor);
    if (plane) {
        removeFromDetachedList(plane);
        attachNodeToAnchor(plane, anchor);
    } else {
        _detachedAnchors.push_back(anchor);
    }
}

std::shared_ptr<VROARPlaneNode> VROARComponentManager::findDetachedPlane(std::shared_ptr<VROARAnchor> anchor) {
    std::vector<std::shared_ptr<VROARPlaneNode>>::iterator it;
    for (it = _detachedPlanes.begin(); it < _detachedPlanes.end(); it++) {
        std::shared_ptr<VROARPlaneNode> candidate = *it;
        if (candidate->hasRequirementsFulfilled(anchor)) {
            return candidate;
        }
    }
    return nullptr;
}

void VROARComponentManager::attachNodeToAnchor(std::shared_ptr<VROARNode> node, std::shared_ptr<VROARAnchor> anchor) {
    anchor->setARNode(node);
    node->setAnchor(anchor);
    node->onARAnchorAttached();
    
    notifyAnchorWasUpdated(anchor);
}

void VROARComponentManager::removeFromDetachedList(std::shared_ptr<VROARPlaneNode> plane) {
    _detachedPlanes.erase(std::remove_if(_detachedPlanes.begin(), _detachedPlanes.end(),
                                         [plane](std::shared_ptr<VROARPlaneNode> candidate) {
                                             return candidate == plane;
                                         }), _detachedPlanes.end());
}

void VROARComponentManager::removeFromDetachedList(std::shared_ptr<VROARAnchor> anchor) {
    _detachedAnchors.erase(std::remove_if(_detachedAnchors.begin(), _detachedAnchors.end(),
                                          [anchor](std::shared_ptr<VROARAnchor> candidate) {
                                              return candidate == anchor;
                                          }), _detachedAnchors.end());
}

void VROARComponentManager::notifyAnchorWasDetected(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARComponentManagerDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorWasDetected(anchor);
    }
}

void VROARComponentManager::notifyAnchorWasUpdated(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARComponentManagerDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorWasUpdated(anchor);
    }
}

void VROARComponentManager::notifyAnchorWasRemoved(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARComponentManagerDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorWasRemoved(anchor);
    }
}
