//
//  VROARDeclarativeSession.cpp
//  ViroKit
//
//  Created by Raj Advani on 11/3/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARDeclarativeSession.h"
#include "VROARDeclarativeNode.h"
#include "VROARDeclarativeImageNode.h"
#include "VROARScene.h"

VROARDeclarativeSession::VROARDeclarativeSession() {
    _constraintMatcher = std::make_shared<VROARConstraintMatcher>();
}

void VROARDeclarativeSession::init() {
    _constraintMatcher->setDelegate(shared_from_this());
}

VROARDeclarativeSession::~VROARDeclarativeSession() {
    
}

void VROARDeclarativeSession::setDelegate(std::shared_ptr<VROARDeclarativeSessionDelegate> delegate) {
    _delegate = delegate;
}

void VROARDeclarativeSession::setARSession(std::shared_ptr<VROARSession> session) {
    _arSession = session;
    // load any ARImageDatabase first because it wipes out the existing DB, and then adds any
    // previously individually added ARImageTargets.
    if (_arImageDatabase) {
        session->loadARImageDatabase(_arImageDatabase);
    }
    for (auto it = _imageTargets.begin(); it < _imageTargets.end(); it++) {
        session->addARImageTarget(*it);
    }
    for (auto it = _objectTargets.begin(); it < _objectTargets.end(); it++) {
        session->addARObjectTarget(*it);
    }
}

void VROARDeclarativeSession::loadARImageDatabase(std::shared_ptr<VROARImageDatabase> arImageDatabase) {
    if (arImageDatabase) {
        _arImageDatabase = arImageDatabase;
        std::shared_ptr<VROARSession> arSession = _arSession.lock();
        if (arSession) {
            arSession->loadARImageDatabase(_arImageDatabase);
        }
    }
}

void VROARDeclarativeSession::unloadARImageDatabase() {
    std::shared_ptr<VROARSession> arSession = _arSession.lock();
    if (arSession && _arImageDatabase) {
        arSession->unloadARImageDatabase();
        _arImageDatabase = nullptr;
    }
}

void VROARDeclarativeSession::addARImageTarget(std::shared_ptr<VROARImageTarget> target) {
    if (target) {
        _imageTargets.push_back(target);
        std::shared_ptr<VROARSession> arSession = _arSession.lock();
        if (arSession) {
            arSession->addARImageTarget(target);
        }
    }
}

void VROARDeclarativeSession::removeARImageTarget(std::shared_ptr<VROARImageTarget> target) {

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

void VROARDeclarativeSession::addARObjectTarget(std::shared_ptr<VROARObjectTarget> target) {
    if (target) {
        _objectTargets.push_back(target);
        std::shared_ptr<VROARSession> arSession = _arSession.lock();
        if (arSession) {
            arSession->addARObjectTarget(target);
        }
    }
}

void VROARDeclarativeSession::removeARObjectTarget(std::shared_ptr<VROARObjectTarget> target) {
    if (target) {
        _objectTargets.erase(
                            std::remove_if(_objectTargets.begin(), _objectTargets.end(),
                                           [target](std::shared_ptr<VROARObjectTarget> candidate) {
                                               return candidate == target;
                                           }), _objectTargets.end());

        std::shared_ptr<VROARSession> arSession = _arSession.lock();
        if (arSession) {
            arSession->removeARObjectTarget(target);
        }
    }
}


void VROARDeclarativeSession::addARNode(std::shared_ptr<VROARDeclarativeNode> node) {
    // TODO: figure out a way to make ARNode simply not be visible at start.
    // call this once, because when it planes are first added they should not be visible.
    node->setAttached(false);
    _nodes.push_back(node);
    if (_constraintMatcher) {
        _constraintMatcher->addARNode(node);
    }
}

void VROARDeclarativeSession::removeARNode(std::shared_ptr<VROARDeclarativeNode> node) {
    node->setAttached(false);
    if (_constraintMatcher) {
        _constraintMatcher->removeARNode(node);
    }
    
    _nodes.erase(
                 std::remove_if(_nodes.begin(), _nodes.end(),
                                [node](std::shared_ptr<VROARDeclarativeNode> candidate) {
                                    return candidate == node;
                                }), _nodes.end());

}

void VROARDeclarativeSession::updateARNode(std::shared_ptr<VROARDeclarativeNode> node) {
    if (_constraintMatcher) {
        _constraintMatcher->updateARNode(node);
    }
}

void VROARDeclarativeSession::sceneWillAppear() {
    for (auto it = _nodes.begin(); it < _nodes.end(); it++) {
        _constraintMatcher->addARNode(*it);
    }

    std::shared_ptr<VROARSession> arSession = _arSession.lock();
    if (arSession) {
        for (auto it = _imageTargets.begin(); it < _imageTargets.end(); it++) {
            arSession->addARImageTarget(*it);
        }
        for (auto it = _objectTargets.begin(); it < _objectTargets.end(); it++) {
            arSession->addARObjectTarget(*it);
        }
    }
}

void VROARDeclarativeSession::sceneWillDisappear() {
    _constraintMatcher->detachAllNodes(_nodes);
    
    std::shared_ptr<VROARSession> arSession = _arSession.lock();
    if (arSession) {
        if (_arImageDatabase) {
            arSession->loadARImageDatabase(_arImageDatabase);
        }
        for (auto it = _imageTargets.begin(); it < _imageTargets.end(); it++) {
            arSession->removeARImageTarget(*it);
        }
        for (auto it = _objectTargets.begin(); it < _objectTargets.end(); it++) {
            arSession->removeARObjectTarget(*it);
        }
    }
}

#pragma mark - VROARSessionDelegate Implementation

void VROARDeclarativeSession::anchorWasDetected(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARDeclarativeSessionDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorWasDetected(anchor);
    }
    _constraintMatcher->anchorWasDetected(anchor);
}

void VROARDeclarativeSession::anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARDeclarativeSessionDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorWillUpdate(anchor);
    }
}

void VROARDeclarativeSession::anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARDeclarativeSessionDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorDidUpdate(anchor);
    }
    _constraintMatcher->anchorDidUpdate(anchor);
}

void VROARDeclarativeSession::anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARDeclarativeSessionDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorWasRemoved(anchor);
    }
    _constraintMatcher->anchorWasRemoved(anchor);
}

#pragma mark - VROARConstraintMatcherDelegate Implementation

void VROARDeclarativeSession::anchorWasAttached(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARDeclarativeSessionDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorDidUpdate(anchor);
    }
}

void VROARDeclarativeSession::anchorWasDetached(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARDeclarativeSessionDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->anchorWasRemoved(anchor);
    }
}
