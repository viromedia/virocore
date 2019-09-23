//
//  VROARConstraintMatcher.cpp
//  ViroKit
//
//  Created by Andy Chu on 6/16/17.
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

#include "VROARConstraintMatcher.h"
#include "VROLog.h"
#include "VROARDeclarativeNode.h"
#include "VROARAnchor.h"

void VROARConstraintMatcher::addARNode(std::shared_ptr<VROARDeclarativeNode> node) {
    processDetachedNode(node);
}

void VROARConstraintMatcher::updateARNode(std::shared_ptr<VROARDeclarativeNode> node) {
    std::shared_ptr<VROARAnchor> anchor = node->getAnchor();

    // if the node has an anchor, then see if the anchor still fulfills the node's requirements
    if (anchor) {
        // if there's an ID on the node, then don't check if the node->hasRequirementsFulfilled
        // which is used for "auto" assignment through minWidth/height, a node w/ id means that the
        // user intends for it to be manually assigned (ignoring minWidth/Height).
        if (!node->getAnchorId().empty()) {
            if (node->getAnchorId() == anchor->getId()) {
                return;
            }
        } else if (node->hasRequirementsFulfilled(anchor)) {
            return;
        }
        // notify the node of detachment, detach anchor & node
        node->onARAnchorRemoved();
        anchor->setARNode(nullptr);

        processDetachedAnchor(anchor);
    }

    processDetachedNode(node);
}

void VROARConstraintMatcher::removeARNode(std::shared_ptr<VROARDeclarativeNode> node) {
    std::shared_ptr<VROARAnchor> anchor = node->getAnchor();
    if (anchor) {
        // detach the node from anchor
        anchor->setARNode(nullptr);

        processDetachedAnchor(anchor);
    } else {
        removeFromDetachedList(node);
    }
}

void VROARConstraintMatcher::setDelegate(std::shared_ptr<VROARConstraintMatcherDelegate> delegate) {
    _delegate = delegate;
}

void VROARConstraintMatcher::detachAllNodes(std::vector<std::shared_ptr<VROARDeclarativeNode>> nodes) {
    std::vector<std::shared_ptr<VROARDeclarativeNode>>::iterator it;
    for (it = nodes.begin(); it < nodes.end(); it++) {
        std::shared_ptr<VROARDeclarativeNode> node = *it;
        
        // if the node had an anchor, then add it back to _detachedAnchors
        std::shared_ptr<VROARAnchor> anchor = node->getAnchor();
        if (anchor) {
            anchor->setARNode(nullptr);
            _detachedAnchors.push_back(anchor);
        }
    }
}

#pragma mark - Forwarded ARSessionDelegate callbacks from ARScene

void VROARConstraintMatcher::anchorWasDetected(std::shared_ptr<VROARAnchor> anchor) {
    _nativeAnchorMap[anchor->getId()] = anchor;
    processDetachedAnchor(anchor);
}

void VROARConstraintMatcher::anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARDeclarativeNode> arNode = std::dynamic_pointer_cast<VROARDeclarativeNode>(anchor->getARNode());
    if (arNode) {
        if (arNode->hasRequirementsFulfilled(anchor)) {
            arNode->onARAnchorUpdated();
            return;
        }
        // if the updated anchor doesn't fulfill the node's requirement, notify that it was removed
        // before attempting to reattach it to another anchor.
        arNode->onARAnchorRemoved();
        arNode->setAnchor(nullptr);
        processDetachedNode(arNode);
    }
    processDetachedAnchor(anchor);
}

void VROARConstraintMatcher::anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARDeclarativeNode> node = std::dynamic_pointer_cast<VROARDeclarativeNode>(anchor->getARNode());
    // if anchor was attached to a node, notify the node that anchor was removed then "handle" the detached node.
    if (node) {
        node->onARAnchorRemoved();
        node->setAnchor(nullptr);
        processDetachedNode(node);
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

std::shared_ptr<VROARAnchor> VROARConstraintMatcher::getAnchorFromId(std::string id) {
    auto it = _nativeAnchorMap.find(id);
    if (it != _nativeAnchorMap.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}


std::shared_ptr<VROARAnchor> VROARConstraintMatcher::findDetachedAnchor(std::shared_ptr<VROARDeclarativeNode> node) {
    std::vector<std::shared_ptr<VROARAnchor>>::iterator it;
    for (it = _detachedAnchors.begin(); it < _detachedAnchors.end(); it++) {
        if (node->hasRequirementsFulfilled(*it)) {
            return *it;
        }
    }
    return nullptr;
}

void VROARConstraintMatcher::processDetachedNode(std::shared_ptr<VROARDeclarativeNode> node) {
    // before we process, we need to remove the node from any list it might be in, because
    // if we fail, then we'll accidentally add it back in again!
    removeFromDetachedList(node);

    std::string anchorId = node->getAnchorId();
    if (!anchorId.empty()) {
        // first find if there's an anchor!
        std::shared_ptr<VROARAnchor> anchor = getAnchorFromId(anchorId);
        if (anchor) {
            removeFromDetachedList(anchor);
            std::shared_ptr<VROARDeclarativeNode> oldNode = std::dynamic_pointer_cast<VROARDeclarativeNode>(anchor->getARNode());
            if (oldNode) {
                if (oldNode->getAnchorId() == anchor->getId()) {
                    // if the oldNode shared the same ID, add the node to the _detachedNodesWithID
                    // vector so if the oldNode is ever detached, we can attach it to the anchor
                    _detachedNodesWithID.push_back(node);
                    return;
                }
                // detach the oldNode and process it as detached node if it doesn't have the same ID
                oldNode->setAnchor(nullptr);
                oldNode->onARAnchorRemoved();
                processDetachedNode(oldNode);
            }
            // attach the given node to the anchor that shares its ID.
            attachNodeToAnchor(node, anchor);
        } else {
            _detachedNodesWithID.push_back(node);
        }
    } else {
        std::shared_ptr<VROARAnchor> anchor = findDetachedAnchor(node);
        if (anchor) {
            removeFromDetachedList(anchor);
            attachNodeToAnchor(node, anchor);
        } else {
            _detachedNodes.push_back(node);
        }
    }
}

void VROARConstraintMatcher::processDetachedAnchor(std::shared_ptr<VROARAnchor> anchor) {
    // before we process, we need to remove the node from any list it might be in, because
    // if we fail, then we'll accidentally add it back in again!
    removeFromDetachedList(anchor);

    std::shared_ptr<VROARDeclarativeNode> node = findDetachedNode(anchor);
    if (node) {
        removeFromDetachedList(node);
        attachNodeToAnchor(node, anchor);
    } else {
        _detachedAnchors.push_back(anchor);
    }
}

std::shared_ptr<VROARDeclarativeNode> VROARConstraintMatcher::findDetachedNode(std::shared_ptr<VROARAnchor> anchor) {
    std::vector<std::shared_ptr<VROARDeclarativeNode>>::iterator it;
    // first check the nodes with ID!
    for (it = _detachedNodesWithID.begin(); it < _detachedNodesWithID.end(); it++) {
        std::shared_ptr<VROARDeclarativeNode> candidate = *it;
        if (candidate->getAnchorId() == anchor->getId()) {
            return candidate;
        }
    }

    for (it = _detachedNodes.begin(); it < _detachedNodes.end(); it++) {
        std::shared_ptr<VROARDeclarativeNode> candidate = *it;
        if (candidate->hasRequirementsFulfilled(anchor)) {
            return candidate;
        }
    }
    return nullptr;
}

void VROARConstraintMatcher::attachNodeToAnchor(std::shared_ptr<VROARDeclarativeNode> node, std::shared_ptr<VROARAnchor> anchor) {
    anchor->setARNode(node);
    node->setAnchor(anchor);
    node->onARAnchorAttached();
    notifyAnchorWasAttached(anchor);
}

void VROARConstraintMatcher::removeFromDetachedList(std::shared_ptr<VROARDeclarativeNode> node) {
    _detachedNodes.erase(std::remove_if(_detachedNodes.begin(), _detachedNodes.end(),
                                         [node](std::shared_ptr<VROARDeclarativeNode> candidate) {
                                             return candidate == node;
                                         }), _detachedNodes.end());

    // check the other list too!
    _detachedNodesWithID.erase(std::remove_if(_detachedNodesWithID.begin(), _detachedNodesWithID.end(),
                                               [node](std::shared_ptr<VROARDeclarativeNode> candidate) {
                                                   return candidate == node;
                                               }), _detachedNodesWithID.end());
}

void VROARConstraintMatcher::removeFromDetachedList(std::shared_ptr<VROARAnchor> anchor) {
    _detachedAnchors.erase(std::remove_if(_detachedAnchors.begin(), _detachedAnchors.end(),
                                          [anchor](std::shared_ptr<VROARAnchor> candidate) {
                                              return candidate == anchor;
                                          }), _detachedAnchors.end());
}

void VROARConstraintMatcher::notifyAnchorWasAttached(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARConstraintMatcherDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorWasAttached(anchor);
    }
}

/*
 Call this function IFF the anchor was removed from the AR subsystem!
 */
void VROARConstraintMatcher::notifyAnchorWasDetached(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARConstraintMatcherDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorWasDetached(anchor);
    }
}
