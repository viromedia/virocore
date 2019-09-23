//
//  VROARImperativeSession.cpp
//  ViroKit
//
//  Created by Raj Advani on 11/5/17.
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

#include "VROARImperativeSession.h"
#include "VROARAnchor.h"
#include "VROARNode.h"
#include "VROARScene.h"

VROARImperativeSession::VROARImperativeSession(std::shared_ptr<VROARScene> scene) : _scene(scene) {
    
}

VROARImperativeSession::~VROARImperativeSession() {
    
}

void VROARImperativeSession::setARSession(std::shared_ptr<VROARSession> session) {
    _arSession = session;
    // load any ARImageDatabase first because it wipes out the existing DB, and then adds any
    // previously individually added ARImageTargets.
    if (_arImageDatabase) {
        session->loadARImageDatabase(_arImageDatabase);
    }
    for (auto it = _imageTargets.begin(); it < _imageTargets.end(); it++) {
        session->addARImageTarget(*it);
    }
}

void VROARImperativeSession::loadARImageDatabase(std::shared_ptr<VROARImageDatabase> arImageDatabase) {
    if (arImageDatabase) {
        _arImageDatabase = arImageDatabase;
        std::shared_ptr<VROARSession> arSession = _arSession.lock();
        if (arSession) {
            arSession->loadARImageDatabase(arImageDatabase);
        }
    }
}

void VROARImperativeSession::unloadARImageDatabase() {
    std::shared_ptr<VROARSession> arSession = _arSession.lock();
    if (arSession && _arImageDatabase) {
        arSession->unloadARImageDatabase();
        _arImageDatabase = nullptr;
    }
}

void VROARImperativeSession::addARImageTarget(std::shared_ptr<VROARImageTarget> target) {
    if (target) {
        _imageTargets.push_back(target);
        std::shared_ptr<VROARSession> arSession = _arSession.lock();
        if (arSession) {
            arSession->addARImageTarget(target);
        }
    }
}

void VROARImperativeSession::removeARImageTarget(std::shared_ptr<VROARImageTarget> target) {

    if (target) {
        _imageTargets.erase(
                std::remove_if(_imageTargets.begin(), _imageTargets.end(),
                               [target](std::shared_ptr<VROARImageTarget> candidate) {
                                   return candidate == target;
                               }), _imageTargets.end());

        std::shared_ptr<VROARSession> arSession = _arSession.lock();
        if (arSession) {
            arSession->removeARImageTarget(target);
        }
    }
}


void VROARImperativeSession::anchorWasDetected(std::shared_ptr<VROARAnchor> anchor) {
    // If the anchor already has a node, use that one (this is the case with manual
    // anchors, where we set the ARNode beforehand)
    std::shared_ptr<VROARNode> node = anchor->getARNode();
    if (!node) {
        node = std::make_shared<VROARNode>();
        anchor->setARNode(node);
    }
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
