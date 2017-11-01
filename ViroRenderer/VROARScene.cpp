//
//  VROARScene.cpp
//  ViroKit
//
//  Created by Andy Chu on 6/13/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARScene.h"
#include "VROARAnchor.h"
#include "VROPortal.h"

void VROARScene::addNode(std::shared_ptr<VRONode> node) {
    getRootNode()->addChildNode(node);
}

void VROARScene::setARSession(std::shared_ptr<VROARSession> arSession) {
    _arSession = arSession;

    // If there wasn't already a emitter, then reset _displayPointCloud
    // to run through the creation/addition logic now that the session is set.
    if (!_pointCloudEmitter) {
        displayPointCloud(_displayPointCloud);
    }
}

void VROARScene::setARComponentManager(std::shared_ptr<VROARComponentManager> arComponentManager) {
    _arComponentManager = arComponentManager;
    _arComponentManager->setDelegate(std::dynamic_pointer_cast<VROARComponentManagerDelegate>(shared_from_this()));
    std::vector<std::shared_ptr<VROARPlaneNode>>::iterator it;
    for (it = _planes.begin(); it < _planes.end(); it++) {
        _arComponentManager->addARPlane(*it);
    }
}

void VROARScene::setDriver(std::shared_ptr<VRODriver> driver) {
    _driver = driver;

    // If there wasn't already a emitter, then reset _displayPointCloud
    // to run through the creation/addition logic now that the driver is set.
    if (!_pointCloudEmitter) {
        displayPointCloud(_displayPointCloud);
    }
}

void VROARScene::displayPointCloud(bool displayPointCloud) {
    _displayPointCloud = displayPointCloud;

    // If we should have an emitter and none exists, try to create one...
    if (_displayPointCloud && !_pointCloudEmitter) {
        _pointCloudEmitter = createPointCloudEmitter();
    }

    // If we had an emitter, then add or remove it.
    if (_pointCloudEmitter) {
        if (_displayPointCloud) {
            VROScene::addParticleEmitter(_pointCloudEmitter);
        } else {
            VROScene::removeParticleEmitter(_pointCloudEmitter);
            _pointCloudEmitter->clearParticles();
        }
    }
}

std::shared_ptr<VROPointCloudEmitter> VROARScene::createPointCloudEmitter() {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    std::shared_ptr<VROARSession> arSession = _arSession.lock();
    if (!driver || !arSession) {
        return nullptr;
    }
    
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    _rootNode->addChildNode(node);

    return std::make_shared<VROPointCloudEmitter>(driver, node, arSession);
}

void VROARScene::setDelegate(std::shared_ptr<VROARSceneDelegate> delegate) {
    _delegate = delegate;
    if (delegate && _hasTrackingInitialized) {
        delegate->onTrackingInitialized();
    }
}

void VROARScene::trackingHasInitialized() {
    std::shared_ptr<VROARSceneDelegate> delegate = _delegate.lock();
    
    // if delegate hasn't yet been set, then just store that tracking was initialized.
    if (!delegate) {
        _hasTrackingInitialized = true;
        return;
    }
    
    // if delegate was initialized, then only notify if tracking hasn't yet been initialized
    if(!_hasTrackingInitialized) {
        _hasTrackingInitialized = true;
        if (delegate) {
            delegate->onTrackingInitialized();
        }
    }
}

void VROARScene::updateAmbientLight(float intensity, float colorTemperature) {
    std::shared_ptr<VROARSceneDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->onAmbientLightUpdate(intensity, colorTemperature);
    }
}


void VROARScene::willAppear() {
    if (_arComponentManager) {
        _arComponentManager->setDelegate(std::dynamic_pointer_cast<VROARComponentManagerDelegate>(shared_from_this()));
        std::vector<std::shared_ptr<VROARPlaneNode>>::iterator it;
        for (it = _planes.begin(); it < _planes.end(); it++) {
            _arComponentManager->addARPlane(*it);
        }
    }
}

void VROARScene::willDisappear() {
    if (_arComponentManager) {
        _arComponentManager->clearAllPlanes(_planes);
    }
}

void VROARScene::addARPlane(std::shared_ptr<VROARPlaneNode> plane) {
    // TODO: figure out a way to make ARNode simply not be visible at start.
    // call this once, because when it planes are first added they should not be visible.
    plane->setIsAttached(false);
    _planes.push_back(plane);
    if (_arComponentManager) {
        _arComponentManager->addARPlane(plane);
    }
}

void VROARScene::removeARPlane(std::shared_ptr<VROARPlaneNode> plane) {
    plane->setIsAttached(false);
    if (_arComponentManager) {
        _arComponentManager->removeARPlane(plane);
    }
    _planes.erase(
                   std::remove_if(_planes.begin(), _planes.end(),
                                  [plane](std::shared_ptr<VROARPlaneNode> candidate) {
                                      return candidate == plane;
                                  }), _planes.end());
}

void VROARScene::updateARPlane(std::shared_ptr<VROARPlaneNode> plane) {
    if (_arComponentManager) {
        _arComponentManager->updateARPlane(plane);
    }
}

#pragma mark - VROARComponentManagerDelegate Implementation

void VROARScene::anchorWasDetected(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARSceneDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->onAnchorFound(anchor);
    }
}

void VROARScene::anchorWasUpdated(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARPlaneAnchor> plane = std::dynamic_pointer_cast<VROARPlaneAnchor>(anchor);
    std::shared_ptr<VROARSceneDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->onAnchorUpdated(anchor);
    }
}

void VROARScene::anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARSceneDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->onAnchorRemoved(anchor);
    }
}
