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
#include <VROARImageTargetAndroid.h>
#include <VROImageAndroid.h>
#include "VROARHitTestResult.h"

VROARSessionARCore::VROARSessionARCore(std::shared_ptr<VRODriverOpenGL> driver) :
    VROARSession(VROTrackingType::DOF6, VROWorldAlignment::Gravity),
    _lightingMode(arcore::LightingMode::AmbientIntensity),
    _planeFindingMode(arcore::PlaneFindingMode::Horizontal),
    _updateMode(arcore::UpdateMode::Blocking),
    _cloudAnchorMode(arcore::CloudAnchorMode::Disabled),
    _cameraTextureId(0),
    _displayRotation(VROARDisplayRotation::R0),
    _rotatedImageDataLength(0),
    _rotatedImageData(nullptr) {

    _session = nullptr;
    _frame = nullptr;

    if (getImageTrackingImpl() == VROImageTrackingImpl::Viro) {
        _arTrackingSession = std::make_shared<VROARTrackingSession>();
    }
    _frameCount = 0;
    _hasTrackingSessionInitialized = false;
}

void VROARSessionARCore::setARCoreSession(arcore::Session *session) {
    _session = session;

    if (getImageTrackingImpl() == VROImageTrackingImpl::ARCore) {
        _currentARCoreImageDatabase = _session->createAugmentedImageDatabase();
    }

    _frame = _session->createFrame();
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
    _session->setCameraTextureName(_cameraTextureId);

    // (re)initialize the tracking session if the camera texture is (re)created
    initTrackingSession();
}

VROARSessionARCore::~VROARSessionARCore() {
    if (_frame) {
        delete (_frame);
    }

    if (_session != nullptr) {
        pinfo("Destroying ARCore session");
        delete (_session);
    }
    if (_rotatedImageData != nullptr) {
        free (_rotatedImageData);
    }

    if (_currentARCoreImageDatabase != nullptr) {
        delete(_currentARCoreImageDatabase);
    }
}

#pragma mark - VROARSession implementation

void VROARSessionARCore::run() {
    if (_session != nullptr) {
        _session->resume();
        pinfo("AR session resumed");
    }
    else {
        pinfo("AR session not resumed: has not yet been configured");
    }
}

void VROARSessionARCore::pause() {
    if (_session != nullptr) {
        _session->pause();
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
                _planeFindingMode = arcore::PlaneFindingMode::Disabled;
                break;
            case VROAnchorDetection::PlanesHorizontal:
                _planeFindingMode = arcore::PlaneFindingMode::Horizontal;
                break;
            default:
                _planeFindingMode = arcore::PlaneFindingMode::Horizontal;
                break;
        }
    }

    if (types.size() == 0) {
        _planeFindingMode = arcore::PlaneFindingMode::Disabled;
    }
    return updateARCoreConfig();
}

void VROARSessionARCore::setCloudAnchorProvider(VROCloudAnchorProvider provider) {
    if (provider == VROCloudAnchorProvider::None) {
        _cloudAnchorMode = arcore::CloudAnchorMode::Disabled;
    }
    else {
        _cloudAnchorMode = arcore::CloudAnchorMode::Enabled;
    }
    updateARCoreConfig();
}

void VROARSessionARCore::setDisplayGeometry(VROARDisplayRotation rotation, int width, int height) {
    _width = width;
    _height = height;
    _displayRotation = rotation;
    if (_session) {
        _session->setDisplayGeometry((int) rotation, width, height);
    }

    // Post to the renderer thread because this function is likely called on the Android Main thread.
    VROPlatformDispatchAsyncRenderer([this](){
        // re-initialize the tracking session if the display geometry resets
        initTrackingSession();
    });
}

void VROARSessionARCore::enableTracking(bool shouldTrack) {
    if (getImageTrackingImpl() == VROImageTrackingImpl::Viro) {
        _arTrackingSession->enableTracking(shouldTrack);
    }
}

bool VROARSessionARCore::configure(arcore::LightingMode lightingMode, arcore::PlaneFindingMode planeFindingMode,
                                   arcore::UpdateMode updateMode, arcore::CloudAnchorMode cloudAnchorMode) {
    _lightingMode = lightingMode;
    _planeFindingMode = planeFindingMode;
    _updateMode = updateMode;
    _cloudAnchorMode = cloudAnchorMode;

    return updateARCoreConfig();
}

bool VROARSessionARCore::updateARCoreConfig() {
    passert_msg(_session != nullptr, "ARCore must be installed before configuring session");

    arcore::Config *config = _session->createConfig(_lightingMode, _planeFindingMode, _updateMode,
                                                    _cloudAnchorMode);

    if (getImageTrackingImpl() == VROImageTrackingImpl::ARCore && _currentARCoreImageDatabase) {
        config->setAugmentedImageDatabase(_currentARCoreImageDatabase);
    }

    bool supported = _session->checkSupported(config);
    if (!supported) {
        pinfo("Failed to configure AR session: configuration not supported");
        return false;
    }
    arcore::ConfigStatus status = _session->configure(config);
    delete (config);

    if (status == arcore::ConfigStatus::Success) {
        pinfo("Successfully configured AR session [lighting %d, planes %d, update %d]",
              _lightingMode, _planeFindingMode, _updateMode);
        _session->resume();
        return true;
    }
    else if (status == arcore::ConfigStatus::UnsupportedConfiguration) {
        pinfo("Failed to configure AR session: configuration not supported");
        return false;
    }
    else if (status == arcore::ConfigStatus::SessionNotPaused) {
        pinfo("Failed to change AR configuration: session must be paused");
        return false;
    }
    else {
        pinfo("Unknown error updating AR configuration");
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
    // on Android we always use Viro tracking implementation
    target->initWithTrackingImpl(getImageTrackingImpl());
    if (getImageTrackingImpl() == VROImageTrackingImpl::Viro) {
        _arTrackingSession->addARImageTarget(target);
    } else if (getImageTrackingImpl() == VROImageTrackingImpl::ARCore) {
        _imageTargets.push_back(target);
        std::weak_ptr<VROARSessionARCore> w_arsession = shared_from_this();
        VROPlatformDispatchAsyncBackground([target, w_arsession] {
            std::shared_ptr<VROARSessionARCore> arsession = w_arsession.lock();
            if (arsession) {
                arsession->addTargetToDatabase(target, arsession->_currentARCoreImageDatabase);
                arsession->updateARCoreConfig();
            }
        });
    }
}

void VROARSessionARCore::removeARImageTarget(std::shared_ptr<VROARImageTarget> target) {
    if (getImageTrackingImpl() == VROImageTrackingImpl::Viro) {
        _arTrackingSession->removeARImageTarget(target);
    } else if (getImageTrackingImpl() == VROImageTrackingImpl::ARCore) {
        // First, we remove the target from the list of targets
        _imageTargets.erase(std::remove_if(_imageTargets.begin(), _imageTargets.end(),
                                           [target](std::shared_ptr<VROARImageTarget> candidate) {
                                               return candidate == target;
                                           }), _imageTargets.end());

        arcore::AugmentedImageDatabase *oldDatabase = _currentARCoreImageDatabase;
        _currentARCoreImageDatabase = _session->createAugmentedImageDatabase();
        std::weak_ptr<VROARSessionARCore> w_arsession = shared_from_this();
        VROPlatformDispatchAsyncBackground([w_arsession, target] {
            std::shared_ptr<VROARSessionARCore> arsession = w_arsession.lock();
            if (arsession) {
                // Now add all the targets back into the database...
                for (int i = 0; i < arsession->_imageTargets.size(); i++) {
                    arsession->addTargetToDatabase(target, arsession->_currentARCoreImageDatabase);
                }
                // then "update" the config with the new target database.
                arsession->updateARCoreConfig();
            }
        });

        delete(oldDatabase);
    }
}

// Note: this function should be called on a background thread (as per guidance by ARCore for the
//       addImageWithPhysicalSize function).
void VROARSessionARCore::addTargetToDatabase(std::shared_ptr<VROARImageTarget> target,
                                             arcore::AugmentedImageDatabase *database) {
    std::shared_ptr<VROARImageTargetAndroid> targetAndroid = std::dynamic_pointer_cast<VROARImageTargetAndroid>(target);
    std::shared_ptr<VROImageAndroid> imageAndroid = std::dynamic_pointer_cast<VROImageAndroid>(targetAndroid->getImage());

    size_t length;
    size_t stride;
    uint8_t *grayscaleImage = imageAndroid->getGrayscaleData(&length, &stride);
    int32_t outIndex;

    int width = imageAndroid->getWidth();
    int height = imageAndroid->getHeight();
    rotateImageForOrientation(&grayscaleImage, &width, &height, &stride, target->getOrientation());

    database->addImageWithPhysicalSize(targetAndroid->getId().c_str(), grayscaleImage,
                                       width, height, (int32_t) stride,
                                       target->getPhysicalWidth(), &outIndex);

    // Free that grayscaleImage now that we're done with it.
    free(grayscaleImage);
}

void VROARSessionARCore::rotateImageForOrientation(uint8_t **grayscaleImage, int *width, int *height,
                                                   size_t *stride, VROImageOrientation orientation) {
    int length = (*width) * (*height);
    if (orientation == VROImageOrientation::Up) {
        *stride = (size_t) *width;
        uint8_t *rotatedImage = new uint8_t[length];
        memcpy(rotatedImage, *grayscaleImage, (size_t) length);
        *grayscaleImage = rotatedImage;
        return;
    } else if (orientation == VROImageOrientation::Down) {
        // if the image is "upside down" then just reverse it...
        *stride = (size_t) *width;
        uint8_t *rotatedImage = new uint8_t[length];
        int index;
        for (int i = 0; i < *height; i++) {
            for (int j = 0; j < *width; j++) {
                index = j + i * *width;
                rotatedImage[index] = (*grayscaleImage)[length - 1 - index];
            }
        }
        *grayscaleImage = rotatedImage;
    } else if (orientation == VROImageOrientation::Left) {
        // if the image is to the "Left" then rotate it CW by 90 degrees
        uint8_t *rotatedImage = new uint8_t[length];

        for (int i = 0; i < *width; i++) {
            for (int j = 0; j < *height; j++) {
                rotatedImage[j + i * *height] = (*grayscaleImage)[(*height - 1 - j) * *width + i];
            }
        }

        // since we rotated, swap the width and height.
        int tempWidth = *width;
        *width = *height;
        *height = tempWidth;

        // set the stride to the new width
        *stride = (size_t) *width;

        // set the grayscaleImage to the rotatedImage.
        *grayscaleImage = rotatedImage;

    } else if (orientation == VROImageOrientation::Right) {
        // if the image is to the "Right" then rotate it CCW by 90 degrees
        uint8_t *rotatedImage = new uint8_t[length];

        for (int i = 0; i < *width; i++) {
            for (int j = 0; j < *height; j++) {
                rotatedImage[j + i * *height] = (*grayscaleImage)[(*width) * (j + 1) - i - 1];
            }
        }

        // since we rotated, swap the width and height.
        int tempWidth = *width;
        *width = *height;
        *height = tempWidth;

        // set the stride to the new width
        *stride = (size_t) *width;

        // set the grayscaleImage to the rotatedImage.
        *grayscaleImage = rotatedImage;
    }
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
    _session->update(_frame);
    _currentFrame = std::make_unique<VROARFrameARCore>(_frame, _viewport, shared_from_this());

    VROARFrameARCore *arFrame = (VROARFrameARCore *) _currentFrame.get();
    processUpdatedAnchors(arFrame);

    // TODO: VIRO-3283 we have a bug where we need to wait a few frames before initializing
    if (getImageTrackingImpl() == VROImageTrackingImpl::Viro) {
        _frameCount++;
        if (!_hasTrackingSessionInitialized && _frameCount == 10) {
            // we need at least 1 frame to initialize the tracking session!
            initTrackingSession();
        }
        _arTrackingSession->updateFrame(arFrame);
    }

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

#pragma mark - VROARTrackingListener Implementation

void VROARSessionARCore::onTrackedAnchorFound(std::shared_ptr<VROARAnchor> anchor) {
    addAnchor(anchor);
}

void VROARSessionARCore::onTrackedAnchorUpdated(std::shared_ptr<VROARAnchor> anchor) {
    updateAnchor(anchor);
}

void VROARSessionARCore::onTrackedAnchorRemoved(std::shared_ptr<VROARAnchor> anchor) {
    removeAnchor(anchor);
}

#pragma mark - Internal Methods

void VROARSessionARCore::initTrackingSession() {
    if (_currentFrame && _synchronizer && _arTrackingSession && getImageTrackingImpl() == VROImageTrackingImpl::Viro) {
        VROARFrameARCore *arFrame = (VROARFrameARCore *) _currentFrame.get();
        _arTrackingSession->init(arFrame, _synchronizer, getCameraTextureId(), _width, _height);
        _arTrackingSession->setListener(shared_from_this());
    }
}

std::shared_ptr<VROARAnchor> VROARSessionARCore::getAnchorForNative(arcore::Anchor *anchor) {
    std::string key = VROStringUtil::toString(anchor->getHashCode());
    auto it = _nativeAnchorMap.find(key);
    if (it != _nativeAnchorMap.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

void VROARSessionARCore::processUpdatedAnchors(VROARFrameARCore *frameAR) {
    arcore::Frame *frame = frameAR->getFrameInternal();

    arcore::AnchorList *anchorList = _session->createAnchorList();
    frame->getUpdatedAnchors(anchorList);
    int anchorsSize = anchorList->size();

    arcore::TrackableList *planeList = _session->createTrackableList();
    frame->getUpdatedTrackables(planeList, arcore::TrackableType::Plane);
    int planeSize = planeList->size();

    // Find all new and updated anchors, update/create new ones and notify this class.
    // Note: this should be 0 until we allow users to add their own anchors to the system.
    if (anchorsSize > 0) {
        for (int i = 0; i < anchorsSize; i++) {
            arcore::Anchor *anchor = anchorList->acquireItem(i);
            std::shared_ptr<VROARAnchor> vAnchor;
            std::string key = VROStringUtil::toString64(anchor->getId());

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

            delete (anchor);
        }
    }

    // Find all new and updated planes, update/create new ones and notify this class
    if (planeSize > 0) {
        for (int i = 0; i < planeSize; i++) {
            arcore::Trackable *trackable = planeList->acquireItem(i);
            arcore::Plane *plane = (arcore::Plane *)trackable;
            arcore::Plane *newPlane = plane->acquireSubsumedBy();
            std::shared_ptr<VROARPlaneAnchor> vAnchor;
            // ARCore doesn't use ID for planes, but rather they simply return the same object, so
            // the hashcodes should be reliable.

            std::string key = VROStringUtil::toString(plane->getHashCode());
            arcore::TrackingState state = trackable->getTrackingState();
            bool currentPlaneIsTracked = state == arcore::TrackingState::Tracking;

            // If the plane wasn't subsumed by a new plane, then don't remove it.
            if (newPlane == NULL && currentPlaneIsTracked) {
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
                delete (newPlane);
            }
            delete (trackable);
        }
    }

    // Process updated/new images if the tracking implementation is ARCore.
    if (getImageTrackingImpl() == VROImageTrackingImpl::ARCore) {
        arcore::TrackableList *imageList = _session->createTrackableList();
        frame->getUpdatedTrackables(imageList, arcore::TrackableType::Image);
        int imageSize = imageList->size();

        if (imageSize > 0) {
            for (int i = 0; i < imageSize; i++) {
                arcore::Trackable *trackable = imageList->acquireItem(i);
                arcore::AugmentedImage *image = (arcore::AugmentedImage *)trackable;
                bool imageIsTracked = trackable->getTrackingState() == arcore::TrackingState::Tracking;
                std::string key(image->getName());

                std::shared_ptr<VROARImageAnchor> vAnchor;
                if (imageIsTracked) {
                    auto it = _nativeAnchorMap.find(key);
                    if (it != _nativeAnchorMap.end()) {
                        vAnchor = std::dynamic_pointer_cast<VROARImageAnchor>(it->second);
                        if (vAnchor) {
                            updateImageAnchorFromARCore(vAnchor, image);
                            updateAnchor(vAnchor);
                        } else {
                            pwarn("[Viro] expected to find an ImageAnchor");
                        }
                    } else {
                        std::shared_ptr<VROARImageTargetAndroid> target;
                        for (int j = 0; j < _imageTargets.size(); j++) {
                            target = std::dynamic_pointer_cast<VROARImageTargetAndroid>(_imageTargets[j]);
                            if (key == target->getId()) {
                                vAnchor = std::make_shared<VROARImageAnchor>(_imageTargets[j]);
                                updateImageAnchorFromARCore(vAnchor, image);
                                vAnchor->setId(target->getId());
                                addAnchor(vAnchor);
                            }
                        }
                    }
                } else {
                    // Subsumed, so remove the plane
                    auto it = _nativeAnchorMap.find(key);
                    if (it != _nativeAnchorMap.end()) {
                        vAnchor = std::dynamic_pointer_cast<VROARImageAnchor>(it->second);
                        if (vAnchor) {
                            removeAnchor(vAnchor);
                        }
                    }
                }
            }
        }
        delete (imageList);
    }

    delete (anchorList);
    delete (planeList);
}

void VROARSessionARCore::updateAnchorFromARCore(std::shared_ptr<VROARAnchor> anchor, arcore::Anchor *anchorAR) {
    arcore::Pose *pose = _session->createPose();
    anchorAR->getPose(pose);

    float mtx[16];
    pose->toMatrix(mtx);
    anchor->setTransform({ mtx });
    delete (pose);
}

void VROARSessionARCore::updatePlaneFromARCore(std::shared_ptr<VROARPlaneAnchor> plane, arcore::Plane *planeAR) {
    arcore::Pose *pose = _session->createPose();
    planeAR->getCenterPose(pose);

    float newTransformMtx[16];
    pose->toMatrix(newTransformMtx);
    VROMatrix4f newTransform(newTransformMtx);
    VROVector3f newTranslation = newTransform.extractTranslation();

    VROMatrix4f oldTransform = plane->getTransform();
    VROVector3f oldTranslation = oldTransform.extractTranslation();

    // Update our plane's transform if it has been previously set.
    if (!oldTranslation.isEqual(VROVector3f())) {
        // Calculate our new center.
        VROVector3f offsetRelativeToPlane = newTranslation - oldTranslation;
        VROVector3f localCenter = VROVector3f(offsetRelativeToPlane.x, 0.0f, offsetRelativeToPlane.z);
        plane->setCenter(localCenter);

        // Calculate the plane's new transform.
        newTransform.translate(localCenter.scale(-1.0));
    }

    plane->setTransform(newTransform);

    switch (planeAR->getPlaneType()) {
        case arcore::PlaneType::HorizontalUpward :
            plane->setAlignment(VROARPlaneAlignment::HorizontalUpward);
            break;
        case arcore::PlaneType::HorizontalDownward :
            plane->setAlignment(VROARPlaneAlignment::HorizontalDownward);
            break;
        default:
            plane->setAlignment(VROARPlaneAlignment::Horizontal);
    }

    float extentX = planeAR->getExtentX();
    float extentZ = planeAR->getExtentZ();
    plane->setExtent(VROVector3f(extentX, 0, extentZ));

    delete (pose);

    // Grab the polygon points from ARCore,
    std::vector<VROVector3f> boundaryVertices;
    float* polygonArray = planeAR->getPolygon();
    int polygonArraySize = planeAR->getPolygonSize();

    if (polygonArraySize > 0) {
        // Parse out polygons from the shape.
        for (int i = 0; i < polygonArraySize; i = i + 2) {
            VROVector3f newPoint;
            newPoint.x = polygonArray[i];
            newPoint.y = 0;
            newPoint.z = polygonArray[i+1];
            boundaryVertices.push_back(newPoint);
        }

        delete [] polygonArray;
    }

    plane->setBoundaryVertices(boundaryVertices);
}

void VROARSessionARCore::updateImageAnchorFromARCore(std::shared_ptr<VROARImageAnchor> imageAnchor,
                                                     arcore::AugmentedImage *imageAR) {
    arcore::Pose *pose = _session->createPose();
    imageAR->getCenterPose(pose);

    float newTransformMtx[16];
    pose->toMatrix(newTransformMtx);
    VROMatrix4f newTransform(newTransformMtx);
    imageAnchor->setTransform(newTransform);

    delete (pose);
}

uint8_t *VROARSessionARCore::getRotatedCameraImageData(int size) {
    if (_rotatedImageData == nullptr || _rotatedImageDataLength != size) {
        free (_rotatedImageData);
        _rotatedImageData = (uint8_t *) malloc(size);
        _rotatedImageDataLength = size;
    }
    return _rotatedImageData;
}
