//
//  VROARSessionARCore.cpp
//  ViroKit
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARSessionARCore.h"
#include "VROARAnchor.h"
#include "VROARPlaneAnchor.h"
#include "VROTexture.h"
#include "VRODriver.h"
#include "VROScene.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROLog.h"
#include <algorithm>
#include <VROCameraTexture.h>
#include "VROPlatformUtil.h"
#include <VROStringUtil.h>
#include "VROARHitTestResult.h"

VROARSessionARCore::VROARSessionARCore(void *context, std::shared_ptr<VRODriverOpenGL> driver) :
    VROARSession(VROTrackingType::DOF6, VROWorldAlignment::Gravity),
    _lightingMode(arcore::config::LightingMode::AmbientIntensity),
    _planeFindingMode(arcore::config::PlaneFindingMode::Horizontal),
    _updateMode(arcore::config::UpdateMode::Blocking),
    _cameraTextureId(0) {

    _session = nullptr;
    _frame = nullptr;
}

void VROARSessionARCore::onARCoreInstalled(void *context) {
    _session = arcore::session::create(context);
    _frame = arcore::frame::create(_session);
}

GLuint VROARSessionARCore::getCameraTextureId() const {
    return _cameraTextureId;
}

void VROARSessionARCore::initCameraTexture(std::shared_ptr<VRODriverOpenGL> driver) {
    // Generate the background texture
    glGenTextures(1, &_cameraTextureId);
    
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, _cameraTextureId);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(
            new VROTextureSubstrateOpenGL(GL_TEXTURE_EXTERNAL_OES, _cameraTextureId, driver, true));
    _background = std::make_shared<VROTexture>(VROTextureType::TextureEGLImage, std::move(substrate));

    passert_msg(_session != nullptr, "ARCore must be installed before setting camera texture");
    arcore::session::setCameraTextureName(_session, _cameraTextureId);
}

VROARSessionARCore::~VROARSessionARCore() {
    if (_frame) {
        arcore::frame::destroy(_frame);
    }

    if (_session != nullptr) {
        pinfo("Destroying ARCore session");
        arcore::session::destroy(_session);
    }
}

#pragma mark - VROARSession implementation

void VROARSessionARCore::run() {
    if (_session != nullptr) {
        arcore::session::resume(_session);
        pinfo("AR session resumed");
    }
    else {
        pinfo("AR session not resumed: has not yet been configured");
    }
}

void VROARSessionARCore::pause() {
    if (_session != nullptr) {
        arcore::session::pause(_session);
        pinfo("AR session paused");
    }
    else {
        pinfo("AR session not paused: has not yet been configured");
    }
}

bool VROARSessionARCore::isReady() const {
    return getScene() != nullptr;
}

void VROARSessionARCore::resetSession(bool resetTracking, bool removeAnchors) {
    return; // no-op
}

bool VROARSessionARCore::setAnchorDetection(std::set<VROAnchorDetection> types) {
    std::set<VROAnchorDetection>::iterator it;
    for (it = types.begin(); it != types.end(); it++) {
        VROAnchorDetection type = *it;
        switch (type) {
            case VROAnchorDetection::None:
                _planeFindingMode = arcore::config::PlaneFindingMode::Disabled;
                break;
            case VROAnchorDetection::PlanesHorizontal:
                _planeFindingMode = arcore::config::PlaneFindingMode::Horizontal;
                break;
        }
    }

    if (types.size() == 0) {
        _planeFindingMode = arcore::config::PlaneFindingMode::Disabled;
    }
    return updateARCoreConfig();
}

void VROARSessionARCore::setDisplayGeometry(int rotation, int width, int height) {
    if (_session != nullptr) {
        arcore::session::setDisplayGeometry(_session, rotation, width, height);
    }
}

bool VROARSessionARCore::configure(arcore::config::LightingMode lightingMode, arcore::config::PlaneFindingMode planeFindingMode,
                                   arcore::config::UpdateMode updateMode) {
    _lightingMode = lightingMode;
    _planeFindingMode = planeFindingMode;
    _updateMode = updateMode;

    return updateARCoreConfig();
}

bool VROARSessionARCore::updateARCoreConfig() {
    passert_msg(_session != nullptr, "ARCore must be installed before configuring session");

    ArConfig *config = arcore::config::create(_lightingMode, _planeFindingMode, _updateMode, _session);
    bool supported = arcore::session::checkSupported(_session, config);
    if (!supported) {
        pinfo("Failed to configure AR session: configuration not supported");
        return false;
    }
    ArStatus status = arcore::session::configure(_session, config);
    arcore::config::destroy(config);

    if (status == AR_SUCCESS) {
        pinfo("Successfully configured AR session [lighting %d, planes %d, update %d]",
              _lightingMode, _planeFindingMode, _updateMode);
        arcore::session::resume(_session);
        return true;
    }
    else if (status == AR_ERROR_UNSUPPORTED_CONFIGURATION) {
        pinfo("Failed to configure AR session: configuration not supported");
        return false;
    }
    else if (status == AR_ERROR_SESSION_NOT_PAUSED) {
        pinfo("Failed to change AR configuration: session must be paused");
        return false;
    }
}

void VROARSessionARCore::setScene(std::shared_ptr<VROScene> scene) {
    VROARSession::setScene(scene);
}

void VROARSessionARCore::setDelegate(std::shared_ptr<VROARSessionDelegate> delegate) {
    VROARSession::setDelegate(delegate);
    // When we add a new delegate, notify it of all the anchors we've found thus far
    if (delegate) {
        for (auto it = _anchors.begin(); it != _anchors.end();it++) {
            delegate->anchorWasDetected(*it);
        }
    }
}

void VROARSessionARCore::addARImageTarget(std::shared_ptr<VROARImageTarget> target) {
    // no-op
}

void VROARSessionARCore::removeARImageTarget(std::shared_ptr<VROARImageTarget> target) {
    // no-op
}

void VROARSessionARCore::addAnchor(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARSessionDelegate> delegate = getDelegate();
    if (delegate) {
        delegate->anchorWasDetected(anchor);
    }

    _anchors.push_back(anchor);
}

void VROARSessionARCore::removeAnchor(std::shared_ptr<VROARAnchor> anchor) {
    _anchors.erase(std::remove_if(_anchors.begin(), _anchors.end(),
                                 [anchor](std::shared_ptr<VROARAnchor> candidate) {
                                     return candidate == anchor;
                                 }), _anchors.end());

    for (auto it = _nativeAnchorMap.begin(); it != _nativeAnchorMap.end();) {
        if (it->second == anchor) {
            // TODO We should remove the anchor from the ARCore session, but unclear
            //      how to do this given just the identifier. Do we create a dummy
            //      ARAnchor and set its identifier?
            //[_session removeAnchor:it->first];
            it = _nativeAnchorMap.erase(it);
        }
        else {
            ++it;
        }
    }
    
    std::shared_ptr<VROARSessionDelegate> delegate = getDelegate();
    if (delegate) {
        delegate->anchorWasRemoved(anchor);
    }
}

void VROARSessionARCore::updateAnchor(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARSessionDelegate> delegate = getDelegate();
    if (delegate) {
        delegate->anchorWillUpdate(anchor);
    }
    anchor->updateNodeTransform();
    if (delegate) {
        delegate->anchorDidUpdate(anchor);
    }
}

std::shared_ptr<VROTexture> VROARSessionARCore::getCameraBackgroundTexture() {
    return _background;
}

std::unique_ptr<VROARFrame> &VROARSessionARCore::updateFrame() {
    arcore::session::update(_session, _frame);
    _currentFrame = std::make_unique<VROARFrameARCore>(_frame, _viewport, shared_from_this());

    VROARFrameARCore *arFrame = (VROARFrameARCore *) _currentFrame.get();
    processUpdatedAnchors(arFrame);
    return _currentFrame;
}

std::unique_ptr<VROARFrame> &VROARSessionARCore::getLastFrame() {
    return _currentFrame;
}

void VROARSessionARCore::setViewport(VROViewport viewport) {
    _viewport = viewport;
}

void VROARSessionARCore::setOrientation(VROCameraOrientation orientation) {
    _orientation = orientation;
}

void VROARSessionARCore::setWorldOrigin(VROMatrix4f relativeTransform) {
    // no-op on Android
}

#pragma mark - Internal Methods

std::shared_ptr<VROARAnchor> VROARSessionARCore::getAnchorForNative(ArAnchor *anchor) {
    std::string key = VROStringUtil::toString(arcore::anchor::getHashCode(anchor));
    auto it = _nativeAnchorMap.find(key);
    if (it != _nativeAnchorMap.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

void VROARSessionARCore::processUpdatedAnchors(VROARFrameARCore *frameAR) {
    ArFrame *frame = frameAR->getFrameInternal();

    ArAnchorList *anchorList = arcore::anchorlist::create(_session);
    arcore::frame::getUpdatedAnchors(frame, _session, anchorList);
    int anchorsSize = arcore::anchorlist::size(anchorList, _session);

    ArTrackableList *planesList = arcore::trackablelist::create(_session);
    arcore::frame::getUpdatedPlanes(frame, _session, planesList);
    int planesSize = arcore::trackablelist::size(planesList, _session);

    // Find all new and updated anchors, update/create new ones and notify this class.
    // Note: this should be 0 until we allow users to add their own anchors to the system.
    if (anchorsSize > 0) {
        for (int i = 0; i < anchorsSize; i++) {
            ArAnchor *anchor = arcore::anchorlist::acquireItem(anchorList, i, _session);
            std::shared_ptr<VROARAnchor> vAnchor;
            std::string key = arcore::anchor::getId(anchor);

            auto it = _nativeAnchorMap.find(key);
            if (it != _nativeAnchorMap.end()) {
                vAnchor = it->second;
                updateAnchorFromARCore(vAnchor, anchor);
                updateAnchor(vAnchor);
            } else {
                vAnchor = std::make_shared<VROARAnchor>();
                _nativeAnchorMap[key] = vAnchor;
                updateAnchorFromARCore(vAnchor, anchor);
                vAnchor->setId(key);
                addAnchor(vAnchor);
            }

            arcore::anchor::release(anchor);
        }
    }

    // Find all new and updated planes, update/create new ones and notify this class
    if (planesSize > 0) {
        for (int i = 0; i < planesSize; i++) {
            ArTrackable *trackable = arcore::trackablelist::acquireItem(planesList, i, _session);
            ArPlane *plane = ArAsPlane(trackable);

            ArPlane *newPlane = arcore::plane::acquireSubsumedBy(plane, _session);

            std::shared_ptr<VROARPlaneAnchor> vAnchor;
            // ARCore doesn't use ID for planes, but rather they simply return the same object, so
            // the hashcodes should be reliable.
            std::string key = VROStringUtil::toString(arcore::plane::getHashCode(plane));

            // If the plane wasn't subsumed by a new plane, then don't remove it.
            if (newPlane == NULL) {
                auto it = _nativeAnchorMap.find(key);
                if (it != _nativeAnchorMap.end()) {
                    vAnchor = std::dynamic_pointer_cast<VROARPlaneAnchor>(it->second);
                    if (vAnchor) {
                        updatePlaneFromARCore(vAnchor, plane);
                        updateAnchor(vAnchor);
                    } else {
                        pwarn("[Viro] expected to find a Plane.");
                    }
                } else {
                    vAnchor = std::make_shared<VROARPlaneAnchor>();
                    _nativeAnchorMap[key] = vAnchor;
                    updatePlaneFromARCore(vAnchor, plane);
                    vAnchor->setId(key);
                    addAnchor(vAnchor);
                }
            } else {
                // Subsumed, so remove the plane
                auto it = _nativeAnchorMap.find(key);
                if (it != _nativeAnchorMap.end()) {
                    vAnchor = std::dynamic_pointer_cast<VROARPlaneAnchor>(it->second);
                    if (vAnchor) {
                        removeAnchor(vAnchor);
                    }
                }
                arcore::trackable::release(ArAsTrackable(newPlane));
            }

            arcore::trackable::release(trackable);
        }
    }

    arcore::anchorlist::destroy(anchorList);
    arcore::trackablelist::destroy(planesList);
}

void VROARSessionARCore::updateAnchorFromARCore(std::shared_ptr<VROARAnchor> anchor, ArAnchor *anchorAR) {
    ArPose *pose = arcore::pose::create(_session);
    arcore::anchor::getPose(anchorAR, _session, pose);
    anchor->setTransform(arcore::pose::toMatrix(pose, _session));
    arcore::pose::destroy(pose);
}

void VROARSessionARCore::updatePlaneFromARCore(std::shared_ptr<VROARPlaneAnchor> plane, ArPlane *planeAR) {
    ArPose *pose = arcore::pose::create(_session);
    arcore::plane::getCenterPose(planeAR, _session, pose);

    VROMatrix4f newTransform = arcore::pose::toMatrix(pose, _session);
    VROVector3f newTranslation = newTransform.extractTranslation();

    VROMatrix4f oldTransform = plane->getTransform();
    VROVector3f oldTranslation = oldTransform.extractTranslation();

    // If the old translation is NOT the zero vector, then we want to preserve the old translation
    // and set the "center" instead.
    if (!oldTranslation.isEqual(VROVector3f())) {
        // set the center to (Position(new) - Position(old).
        plane->setCenter(newTranslation - oldTranslation);
        // translate the newTransform by the difference of (Position(old) - Position(new)) to
        // keep the same oldTranslation.
        newTransform.translate(oldTranslation - newTranslation);
    }

    plane->setTransform(newTransform);

    switch (arcore::plane::getType(planeAR, _session)) {
        case arcore::PlaneType::HorizontalUpward :
            plane->setAlignment(VROARPlaneAlignment::HorizontalUpward);
            break;
        case arcore::PlaneType::HorizontalDownward :
            plane->setAlignment(VROARPlaneAlignment::HorizontalDownward);
            break;
        default:
            plane->setAlignment(VROARPlaneAlignment::NonHorizontal);
    }

    // the center is 0, because in ARCore, planes only have a position (at their center) vs
    // ARKit's (initial) position & center.
    plane->setCenter(VROVector3f());

    float extentX = arcore::plane::getExtentX(planeAR, _session);
    float extentZ = arcore::plane::getExtentZ(planeAR, _session);
    plane->setExtent(VROVector3f(extentX, 0, extentZ));

    arcore::pose::destroy(pose);
}
