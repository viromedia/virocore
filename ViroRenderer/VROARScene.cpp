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
#include "VROARFrame.h"
#include "VROARDeclarativeNode.h"
#include "VROARConstraintMatcher.h"
#include "VROFixedParticleEmitter.h"
#include "VROARImperativeSession.h"
#include "VROARDeclarativeSession.h"
#include "VROMaterial.h"
#include "VROImageUtil.h"
#include "VROARCamera.h"

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
    std::shared_ptr<VROARSession> session = _arSession.lock();
    if (session) {
        _declarativeSession->setARSession(session);
    }
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

void VROARScene::setAnchorDetectionTypes(std::set<VROAnchorDetection> detectionTypes) {
    _detectionTypes = detectionTypes;
    std::shared_ptr<VROARSession> arSession = _arSession.lock();
    if (arSession) {
        arSession->setAnchorDetection(_detectionTypes);
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

    if (_declarativeSession) {
        _declarativeSession->setARSession(arSession);
    }

    arSession->setAnchorDetection(_detectionTypes);
}

void VROARScene::setDriver(std::shared_ptr<VRODriver> driver) {
    _driver = driver;

    // If there wasn't already a emitter, then reset _displayPointCloud
    // to run through the creation/addition logic now that the driver is set.
    if (!_pointCloudEmitter) {
        displayPointCloud(_displayPointCloud);
    }
}

void VROARScene::updateParticles(const VRORenderContext &context) {
    updatePointCloud();
    VROScene::updateParticles(context);
}

void VROARScene::updatePointCloud(){
    if (!_pointCloudEmitter || !_displayPointCloud){
        return;
    }

    std::shared_ptr<VROARSession> arSession = _arSession.lock();
    if (!arSession) {
        return;
    }

    std::unique_ptr<VROARFrame> &frame = arSession->getLastFrame();
    if (!frame) {
        return;
    }

    std::vector<VROVector4f> pointCloudPoints = frame->getPointCloud()->getPoints();
    _pointCloudEmitter->setParticleTransforms(pointCloudPoints);
}

void VROARScene::displayPointCloud(bool displayPointCloud) {
    _displayPointCloud = displayPointCloud;

    // If we should have an emitter and none exists, try to create one...
    if (_displayPointCloud && !_pointCloudEmitter) {
        // Note: the creation could fail!
        initPointCloudEmitter();
    }

    // Point cloud emitter has not yet initialized.
    if (_pointCloudEmitter == nullptr){
        return;
    }

    // Everything has been initialized, add / remove emitter as needed.
    if (_displayPointCloud) {
        _pointCloudNode->setParticleEmitter(_pointCloudEmitter);
        if (_pointCloudNode->getParentNode() == nullptr) {
            addNode(_pointCloudNode);
        }
    } else {
        _pointCloudNode->removeParticleEmitter();
        _pointCloudEmitter->forceClearParticles();
    }
}

void VROARScene::resetPointCloudSurface() {
    if (!_pointCloudEmitter) {
        return;
    }

    // Set the default surface on the point cloud emitter.
    _pointCloudSurface = VROSurface::createSurface(1, 1);
    _pointCloudSurface->getMaterials()[0]->getDiffuse().setTexture(getPointCloudTexture());
    _pointCloudSurface->getMaterials()[0]->setBloomThreshold(-1);
    _pointCloudSurface->getMaterials()[0]->setBlendMode(VROBlendMode::Add);
    _pointCloudEmitter->setParticleSurface(_pointCloudSurface);
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

void VROARScene::initPointCloudEmitter() {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    std::shared_ptr<VROARSession> arSession = _arSession.lock();
    if (!driver || !arSession) {
        return;
    }

    _pointCloudEmitter = std::make_shared<VROFixedParticleEmitter>(driver);
    _pointCloudEmitter->setMaxParticles(_pointCloudMaxPoints);
    _pointCloudEmitter->setParticleScale(_pointCloudSurfaceScale);

    // Set a generic point cloud surface on the emitter if none is given.
    if (!_pointCloudSurface) {
        resetPointCloudSurface();
    } else {
        _pointCloudEmitter->setParticleSurface(_pointCloudSurface);
    }
}

void VROARScene::setDelegate(std::shared_ptr<VROARSceneDelegate> delegate) {
    _delegate = delegate;
    setTrackingState(_currentTrackingState, _currentTrackingStateReason, true);
}

void VROARScene::setTrackingState(VROARTrackingState state, VROARTrackingStateReason reason,
                                  bool force) {
    if (_currentTrackingState == state && _currentTrackingStateReason == reason && !force) {
        return;
    }
    _currentTrackingState = state;
    _currentTrackingStateReason = reason;

    // Notify delegates.
    std::shared_ptr<VROARSceneDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->onTrackingUpdated(state, reason);
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
