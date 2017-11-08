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
#include "VROARSession.h"
#include "VROARDeclarativeNode.h"
#include "VROARConstraintMatcher.h"
#include "VROPointCloudEmitter.h"
#include "VROARImperativeSession.h"
#include "VROARDeclarativeSession.h"

VROARScene::~VROARScene() {
    // no-op, we define this here vs in header because for some reason if you want
    // to dynamic cast from a base to a derived class, the derived class had better
    // override a virtual function, but not in the header. See this link for related
    // issue: https://bytes.com/topic/c/answers/851223-q-strange-dynamic_cast-problem
}

void VROARScene::initDeclarativeSession() {
    passert (_imperativeSession == nullptr);
    _declarativeSession = std::make_shared<VROARDeclarativeSession>();
    _declarativeSession->init();
}

void VROARScene::initImperativeSession() {
    passert (_declarativeSession == nullptr);

    std::shared_ptr<VROARScene> scene = std::dynamic_pointer_cast<VROARScene>(shared_from_this());
    _imperativeSession = std::make_shared<VROARImperativeSession>(scene);
}

std::shared_ptr<VROARSessionDelegate> VROARScene::getSessionDelegate() {
    if (_declarativeSession) {
        return _declarativeSession;
    }
    else {
        return _imperativeSession;
    }
}

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
        // Note: the creation could fail!
        _pointCloudEmitter = createPointCloudEmitter();
        if (_pointCloudEmitter) {
            if (_pointCloudSurface) {
                _pointCloudEmitter->setParticleSurface(_pointCloudSurface);
            }
            _pointCloudEmitter->setMaxParticles(_pointCloudMaxPoints);
            _pointCloudEmitter->setParticleScale(_pointCloudSurfaceScale);
        }
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

void VROARScene::resetPointCloudSurface() {
    _pointCloudSurface = nullptr;
    if (_pointCloudEmitter) {
        _pointCloudEmitter->resetParticleSurface();
    }
}

void VROARScene::setPointCloudSurface(std::shared_ptr<VROSurface> surface) {
    _pointCloudSurface = surface;
    if (_pointCloudEmitter) {
        _pointCloudEmitter->setParticleSurface(surface);
    }
}

void VROARScene::setPointCloudSurfaceScale(VROVector3f scale) {
    _pointCloudSurfaceScale = scale;
    if (_pointCloudEmitter) {
        _pointCloudEmitter->setParticleScale(scale);
    }
}

void VROARScene::setPointCloudMaxPoints(int maxPoints) {
    _pointCloudMaxPoints = maxPoints;
    if (_pointCloudEmitter) {
        _pointCloudEmitter->setMaxParticles(_pointCloudMaxPoints);
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
    if (_declarativeSession) {
        _declarativeSession->sceneWillAppear();
    }
}

void VROARScene::willDisappear() {
    if (_declarativeSession) {
        _declarativeSession->sceneWillDisappear();
    }
}
