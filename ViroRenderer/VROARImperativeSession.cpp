//
//  VROARImperativeSession.cpp
//  ViroKit
//
//  Created by Raj Advani on 11/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARImperativeSession.h"
#include "VROARAnchor.h"
#include "VROARNode.h"
#include "VROARScene.h"

VROARImperativeSession::VROARImperativeSession(std::shared_ptr<VROARScene> scene) : _scene(scene) {
    
}

VROARImperativeSession::~VROARImperativeSession() {
    
}

void VROARImperativeSession::anchorWasDetected(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARNode> node = std::make_shared<VROARNode>();
    anchor->setARNode(node);
    node->setAnchor(anchor);

    std::shared_ptr<VROARScene> scene = _scene.lock();
    if (scene) {
        scene->addNode(node);
    }
    std::shared_ptr<VROARImperativeSessionDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorWasDetected(anchor, node);
    }
}

void VROARImperativeSession::anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARImperativeSessionDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorWillUpdate(anchor, anchor->getARNode());
    }
}

void VROARImperativeSession::anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARImperativeSessionDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorDidUpdate(anchor, anchor->getARNode());
    }
}

void VROARImperativeSession::anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARNode> node = anchor->getARNode();
    node->setAnchor(nullptr);
    anchor->setARNode(nullptr);
    node->removeFromParentNode();
    
    std::shared_ptr<VROARImperativeSessionDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorWasRemoved(anchor, node);
    }
}
