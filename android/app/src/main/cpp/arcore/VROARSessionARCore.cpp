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

static bool kDebugTracking = false;

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

#pragma mark - Lifecycle and Setup

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

    bool planesHorizontal = false;
    bool planesVertical = false;

    for (it = types.begin(); it != types.end(); it++) {
        VROAnchorDetection type = *it;
        switch (type) {
            case VROAnchorDetection::PlanesHorizontal:
                planesHorizontal = true;
                break;
            case VROAnchorDetection::PlanesVertical:
                planesVertical = true;
                break;
        }
    }

    if (planesHorizontal && planesVertical) {
        _planeFindingMode = arcore::PlaneFindingMode::HorizontalAndVertical;
    } else if (planesHorizontal) {
        _planeFindingMode = arcore::PlaneFindingMode::Horizontal;
    } else if (planesVertical) {
        _planeFindingMode = arcore::PlaneFindingMode::Vertical;
    } else {
        _planeFindingMode = arcore::PlaneFindingMode::Disabled;
    }
    return updateARCoreConfig();
}

void VROARSessionARCore::setCloudAnchorProvider(VROCloudAnchorProvider provider) {
    if (provider == VROCloudAnchorProvider::None) {
        _cloudAnchorMode = arcore::CloudAnchorMode::Disabled;
    } else {
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

void VROARSessionARCore::setViewport(VROViewport viewport) {
    _viewport = viewport;
}

void VROARSessionARCore::setOrientation(VROCameraOrientation orientation) {
    _orientation = orientation;
}

void VROARSessionARCore::setWorldOrigin(VROMatrix4f relativeTransform) {
    // no-op on Android
}

#pragma mark - AR Image Targets

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

#pragma mark - Anchors

void VROARSessionARCore::addAnchor(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARAnchorARCore> vAnchor = std::dynamic_pointer_cast<VROARAnchorARCore>(anchor);
    passert (vAnchor);

    // Add the anchor under both its keys: the top-level anchor key and the trackable key.
    // The former keeps anchors we've created and attached to trackables from being treated as
    // "new" anchors in processUpdatedAnchors.
    _nativeAnchorMap[VROStringUtil::toString64(vAnchor->getAnchorInternal()->getId())] = vAnchor;
    _nativeAnchorMap[anchor->getId()] = vAnchor;

    if (kDebugTracking) pinfo("Added new new anchor [%p -- %p]", VROStringUtil::toString64(vAnchor->getAnchorInternal()->getId()).c_str(),
    anchor->getId().c_str());

    std::shared_ptr<VROARSessionDelegate> delegate = getDelegate();
    if (delegate) {
        delegate->anchorWasDetected(anchor);
    }
    _anchors.push_back(vAnchor);
}

void VROARSessionARCore::removeAnchor(std::shared_ptr<VROARAnchor> anchor) {
    _anchors.erase(std::remove_if(_anchors.begin(), _anchors.end(),
                                 [anchor](std::shared_ptr<VROARAnchor> candidate) {
                                     return candidate == anchor;
                                 }), _anchors.end());

    for (auto it = _nativeAnchorMap.begin(); it != _nativeAnchorMap.end();) {
        if (it->second == anchor) {
            it = _nativeAnchorMap.erase(it);
        } else {
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

#pragma mark - AR Frames

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

/*
 This method does most of the ARCore processing. ARCore consists of two concepts: trackable and
 anchor. Trackables are detected real-world objects, like horizontal and vertical planes, or image
 targets. Anchors are virtual objects that are attached to the real world, either relative to a
 trackable or relative to an AR hit result.

 Unlike ARCore, Viro (and ARKit) merge these concepts together: trackables *are* anchors.
 In order to bridge this conceptual difference with ARCore, this method will create one ARCore anchor
 for every ARCore trackable found. It will attach that anchor to the trackable with the trackable's
 center pose.

 We then create a Viro object to correspond to each of these: a VROARAnchorARCore to correspond to
 the anchor we created for the trackable, and another VROARAnchor subclass to correspond to the
 trackable itself. For example, for planes:

 1. ARCore detects a new arcore::Plane
 2. We create an arcore::Anchor attached to the plane (via arcore::Plane->acquireAnchor)
 3. We create Viro object VROARAnchorARCore to correspond to the arcore::Anchor
 4. We create Viro object VROARAnchorPlane to correspond to the arcore::Plane
 5. We associate the VROARAnchorARCore to the VROARAnchorPlane via VRORAAnchorARCore->setTrackable()
 6. We place the VROARAnchorARCore in the _nativeAnchorMap and the _anchors list

 One point of confusion is that both anchors and trackables have their own transformation matrix.
 We use the anchor transformation matrix when determining how to place the ARNodes that we generate
 for each created Anchor. This is for compatibility with cloud anchors: the devices receiving
 the content will only have the anchor transformation.

 All anchors found here are placed in the _nativeAnchorMap. We only place the top-level anchor in
 the map. We do not place the trackable anchors themselves in the map. For each type of anchor we
 use a different key:

 1. For anchors without a trackable, we key by the anchor's ID.
 2. For plane trackables, we key by the anchor's ID *and* by the trackable's pointer address.
    Inserting keys for both the anchor and the trackable ensures that we don't treat the anchor
    we've created for the trackable as a brand new anchor during the next processUpdatedAnchors
    call.
 3. For image trackables, key by the anchor's ID *and* the image's name. Note that keying by the
    image's name has the effect of ensuring we only recognize *one* image of a type at a time.

 Finally, all anchors found are also placed in the _anchors list. As with the _nativeAnchorMap, we
 only place top-level anchors here (not the trackable anchors).
 */
void VROARSessionARCore::processUpdatedAnchors(VROARFrameARCore *frameAR) {
    arcore::Frame *frame = frameAR->getFrameInternal();

    arcore::AnchorList *anchorList = _session->createAnchorList();
    frame->getUpdatedAnchors(anchorList);
    int anchorsSize = anchorList->size();

    // Find all new and updated anchors, update/create new ones and notify this class.
    // The anchors in this list are *both* those that are tied to trackables and those
    // that were created during hit tests and tied to world coordinates. However, we only
    // process anchors that have no trackable here. Anchors with trackables are processed
    // below.
    for (int i = 0; i < anchorsSize; i++) {
        std::shared_ptr<arcore::Anchor> anchor = std::shared_ptr<arcore::Anchor>(anchorList->acquireItem(i));
        std::string key = VROStringUtil::toString64(anchor->getId());
        auto it = _nativeAnchorMap.find(key);

        // Previously found anchor: update
        if (it != _nativeAnchorMap.end()) {
            std::shared_ptr<VROARAnchorARCore> vAnchor = it->second;
            if (kDebugTracking) pinfo("Updating anchor %p with trackable %p", vAnchor.get(),
                  vAnchor->getTrackable().get());

            // If the anchor has a trackable, do not process it (it will be processed with its
            // associated trackable below)
            if (!vAnchor->getTrackable()) {
                passert (anchor->getId() == vAnchor->getAnchorInternal()->getId());
                syncAnchorWithARCore(vAnchor, anchor.get());
                updateAnchor(vAnchor);
            }

        // New anchor, not tied to any trackable
        } else {
            pinfo("Detected new anchor [%p] not tied to any trackable", key.c_str());

            std::shared_ptr<VROARAnchorARCore> vAnchor = std::make_shared<VROARAnchorARCore>(key, anchor, nullptr);
            syncAnchorWithARCore(vAnchor, anchor.get());
            addAnchor(vAnchor);
        }
    }

    arcore::TrackableList *planeList = _session->createTrackableList();
    frame->getUpdatedTrackables(planeList, arcore::TrackableType::Plane);
    int planeSize = planeList->size();

    // Find all new and updated planes and process them. For new planes we will create a
    // corresponding anchor. For updated planes we will update the planes and the anchor.
    // Finally, we remove subsumed planes.
    for (int i = 0; i < planeSize; i++) {
        arcore::Trackable *trackable = planeList->acquireItem(i);
        arcore::Plane * plane = (arcore::Plane *) trackable;
        arcore::Plane * subsumingPlane = plane->acquireSubsumedBy();

        arcore::TrackingState state = trackable->getTrackingState();
        bool currentPlaneIsTracked = (state == arcore::TrackingState::Tracking);

        // ARCore doesn't use ID for planes, but rather they simply return the same object, so
        // the hashcodes (which in this case are pointer addresses) should be reliable
        std::string key = VROStringUtil::toString(plane->getHashCode());

        // The plane was *NOT* subsumed by a new plane and is still tracking:
        // either add or update it
        if (subsumingPlane == NULL && currentPlaneIsTracked) {
            auto it = _nativeAnchorMap.find(key);

            // The plane is old: update it
            if (it != _nativeAnchorMap.end()) {
                std::shared_ptr<VROARAnchorARCore> vAnchor = it->second;

                if (vAnchor) {
                    if (kDebugTracking) pinfo("Updating anchor %p with plane trackable %p", vAnchor.get(),
                          vAnchor->getTrackable().get());

                    std::shared_ptr<VROARPlaneAnchor> vPlane = std::dynamic_pointer_cast<VROARPlaneAnchor>(
                            vAnchor->getTrackable());
                    syncPlaneWithARCore(vPlane, plane);
                    syncAnchorWithARCore(vAnchor, vAnchor->getAnchorInternal().get());
                    updateAnchor(vAnchor);
                } else {
                    pwarn("Anchor processing error: expected to find a plane");
                }

            // The plane is new: add it
            } else {
                pinfo("Detected new anchor tied to plane");

                std::shared_ptr<VROARPlaneAnchor> vPlane = std::make_shared<VROARPlaneAnchor>();
                syncPlaneWithARCore(vPlane, plane);

                // Create a new anchor to correspond with the found plane
                arcore::Pose *pose = _session->createPose();
                plane->getCenterPose(pose);
                std::shared_ptr<arcore::Anchor> anchor = std::shared_ptr<arcore::Anchor>(plane->acquireAnchor(pose));

                std::shared_ptr<VROARAnchorARCore> vAnchor = std::make_shared<VROARAnchorARCore>(key, anchor, vPlane);
                addAnchor(vAnchor);
                delete (pose);
            }

        // The plane has been subsumed or is no longer tracked: detach it and remove it
        } else {
            auto it = _nativeAnchorMap.find(key);
            if (it != _nativeAnchorMap.end()) {
                if (subsumingPlane) {
                    pinfo("Plane %s subsumed: removing", key.c_str());
                } else {
                    pinfo("Plane %s no longer tracked: removing", key.c_str());
                }

                std::shared_ptr<VROARAnchorARCore> vAnchor = it->second;
                if (vAnchor) {
                    vAnchor->getAnchorInternal()->detach();
                    removeAnchor(vAnchor);
                }
            }
            delete (subsumingPlane);
        }
        delete (trackable);
    }

    // Process updated/new images if the tracking implementation is ARCore. This process
    // is virtually identical to how we handle planes above.
    if (getImageTrackingImpl() == VROImageTrackingImpl::ARCore) {

        arcore::TrackableList *imageList = _session->createTrackableList();
        frame->getUpdatedTrackables(imageList, arcore::TrackableType::Image);
        int imageSize = imageList->size();

        for (int i = 0; i < imageSize; i++) {
            arcore::Trackable *trackable = imageList->acquireItem(i);
            arcore::AugmentedImage *image = (arcore::AugmentedImage *) trackable;

            // The name of the image is used for image anchors. This enforces the condition
            // that we only detect each image once
            std::string key(image->getName());

            bool imageIsTracked = (trackable->getTrackingState() == arcore::TrackingState::Tracking);
            if (imageIsTracked) {
                auto it = _nativeAnchorMap.find(key);

                // Old image tracking target: update it
                if (it != _nativeAnchorMap.end()) {
                    std::shared_ptr<VROARAnchorARCore> vAnchor = it->second;
                    std::shared_ptr<VROARImageAnchor> imageAnchor = std::dynamic_pointer_cast<VROARImageAnchor>(
                            vAnchor->getTrackable());

                    if (vAnchor) {
                        if (kDebugTracking) pinfo("Updating anchor %p with image trackable %p", vAnchor.get(),
                              vAnchor->getTrackable().get());

                        symcImageAnchorWithARCore(imageAnchor, image);
                        syncAnchorWithARCore(vAnchor, vAnchor->getAnchorInternal().get());
                        updateAnchor(vAnchor);
                    } else {
                        pwarn("Anchor processing error: expected to find an image anchor");
                    }

                // New image tracking target: add it
                } else {
                    std::shared_ptr<VROARImageTargetAndroid> target;
                    for (int j = 0; j < _imageTargets.size(); j++) {
                        target = std::dynamic_pointer_cast<VROARImageTargetAndroid>(_imageTargets[j]);

                        if (key == target->getId()) {
                            pinfo("Detected new anchor tied to image target [%s]", key.c_str());

                            std::shared_ptr<VROARImageAnchor> vImage = std::make_shared<VROARImageAnchor>(_imageTargets[j]);
                            symcImageAnchorWithARCore(vImage, image);

                            // Create a new anchor to correspond with the found image
                            arcore::Pose *pose = _session->createPose();
                            image->getCenterPose(pose);
                            std::shared_ptr<arcore::Anchor> anchor = std::shared_ptr<arcore::Anchor>(image->acquireAnchor(pose));

                            std::shared_ptr<VROARAnchorARCore> vAnchor = std::make_shared<VROARAnchorARCore>(key, anchor, vImage);
                            addAnchor(vAnchor);
                            delete (pose);
                        }
                    }
                }
            // The image is no longer being tracked: detach and remove it
            } else {
                auto it = _nativeAnchorMap.find(key);
                if (it != _nativeAnchorMap.end()) {
                    pinfo("Image target [%s] has lost tracking, removing", key.c_str());

                    std::shared_ptr<VROARAnchorARCore> vAnchor = it->second;
                    if (vAnchor) {
                        vAnchor->getAnchorInternal()->detach();
                        removeAnchor(vAnchor);
                    }
                }
            }
        }
        delete (imageList);
    }

    delete (anchorList);
    delete (planeList);
}

void VROARSessionARCore::syncAnchorWithARCore(std::shared_ptr<VROARAnchor> anchor,
                                              arcore::Anchor *anchorAR) {
    arcore::Pose *pose = _session->createPose();
    anchorAR->getPose(pose);

    float mtx[16];
    pose->toMatrix(mtx);
    anchor->setTransform({ mtx });
    delete (pose);
}

void VROARSessionARCore::syncPlaneWithARCore(std::shared_ptr<VROARPlaneAnchor> plane,
                                             arcore::Plane *planeAR) {
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
        case arcore::PlaneType ::Vertical :
            plane->setAlignment(VROARPlaneAlignment::Vertical);
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

void VROARSessionARCore::symcImageAnchorWithARCore(std::shared_ptr<VROARImageAnchor> imageAnchor,
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
