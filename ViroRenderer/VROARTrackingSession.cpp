//
//  VROARTrackingSession.cpp
//  ViroKit
//
// Created by Andy Chu on 3/7/18.
//  Copyright © 2017 Viro Media. All rights reserved.
//


#include "VROARTrackingSession.h"
#include "VROARImageAnchor.h"
#include "VROPlatformUtil.h"
#include "VROData.h"
#include <algorithm>

#if VRO_PLATFORM_ANDROID
#include "VROARImageTargetAndroid.h"
#endif

VROARTrackingSession::VROARTrackingSession() {
#if ENABLE_OPENCV
    _tracker = VROARImageTracker::createDefaultTracker();
#endif // ENABLE_OPENCV
    _readyForTracking = true;
    _haveActiveTargets = false;
    _isTextureReaderStarted = false;
    _shouldTrack = true;
}

VROARTrackingSession::~VROARTrackingSession() {
    
}

void VROARTrackingSession::init(VROARFrameARCore *frame,
                                std::shared_ptr<VROFrameSynchronizer> synchronizer,
                                GLuint cameraTextureId, int width, int height) {
    // if we are reinitializing, stop the previous texture reader!
    stopTextureReader();

    _frameSynchronizer = synchronizer;

    _tracker->computeIntrinsicMatrix(width, height);

    _cameraTextureReader = std::make_shared<VROTextureReader>(cameraTextureId,
                                                              width, height,
                                                              width, height,
                                                              1, // check every frame if we're ready
                                                              VROTextureReaderOutputFormat::RGBA8,
            [width, height, this](std::shared_ptr<VROData> data) {
                // check whether or not we should start a new tracking task
                if (!_readyForTracking) {
                    return;
                }

                _readyForTracking = false;
                // convert to cv::Mat now (TODO: should we do this on the background? don't forget to
                // copy the VROData object in that case!)
                cv::Mat tmpFrame(height, width, CV_8UC4, data->getData());
                // transpose the matrix...
                tmpFrame = tmpFrame.t();
                cv::rotate(tmpFrame, tmpFrame, cv::ROTATE_90_COUNTERCLOCKWISE);

                VROPlatformDispatchAsyncBackground([tmpFrame, this]() {
                    // TODO: get intrinsics matrix (ARCore doesn't have this yet!)
                    std::vector<VROARImageTrackerOutput> outputs = _tracker->findTarget(tmpFrame, NULL, _lastCamera);

                    for (int i = 0; i < outputs.size(); i++) {
                        VROARImageTrackerOutput output = outputs[i];
                        // if the output wasn't a "found" output, ignore it.
                        if (!output.found) {
                            continue;
                        }

                        // if the target is one we're going to remove, also ignore it.
                        auto targetIt = _targetsToRemove.find(output.target);
                        if (targetIt != _targetsToRemove.end()) {
                            continue;
                        }

                        bool isUpdate;
                        std::shared_ptr<VROARImageAnchor> imageAnchor;
                        auto targetAnchorIt = _targetAnchorMap.find(output.target);
                        if (targetAnchorIt == _targetAnchorMap.end()) {
                            // call the _listener.onAnchorFound w/ the anchor for output.target
                            imageAnchor = std::make_shared<VROARImageAnchor>(output.target);
                            _targetAnchorMap[output.target] = imageAnchor;
                            isUpdate = false;
                        } else {
                            // call the _listener.onAnchorUpdated w/ the anchor for output.target
                            imageAnchor = targetAnchorIt->second;
                            isUpdate = true;
                        }

                        std::shared_ptr<VROARImageTargetAndroid> imageTargetAndroid =
                                std::dynamic_pointer_cast<VROARImageTargetAndroid>(output.target);
                        imageAnchor->setId(imageTargetAndroid->getId());
                        imageAnchor->setTransform(output.worldTransform);

                        std::shared_ptr<VROARTrackingListener> listener = _weakListener.lock();
                        if (listener) {
                            VROPlatformDispatchAsyncRenderer([isUpdate, listener, imageAnchor](){
                                if (isUpdate) {
                                    listener->onTrackedAnchorUpdated(imageAnchor);
                                } else {
                                    listener->onTrackedAnchorFound(imageAnchor);
                                }
                            });
                        }
                    }
                    // Now that we're done, go back to the renderer thread call flushTargetsToRemove
                    // set the _readyForTracking back to true (we need to make sure that the function
                    // is called before the next tracking task is run!
                    VROPlatformDispatchAsyncRenderer([this](){
                        flushTargetsToRemove();
                        _readyForTracking = true;
                    });
                });
            });

    VROVector3f BL, BR, TL, TR;
    frame->getBackgroundTexcoords(&BL, &BR, &TL, &TR);
    _cameraTextureReader->setTextureCoordinates(BL, BR, TL, TR);
    _cameraTextureReader->init();

    if (_shouldTrack && _haveActiveTargets) {
        startTextureReader();
    }
}

void VROARTrackingSession::updateFrame(VROARFrame *frame) {
    // we're storing this camera, hopefully it's in sync with the callback we get from
    // the texture reader...
    _lastCamera = frame->getCamera();
}

void VROARTrackingSession::setListener(std::shared_ptr<VROARTrackingListener> listener) {
    _weakListener = listener;
}

void VROARTrackingSession::addARImageTarget(std::shared_ptr<VROARImageTarget> target) {
    _imageTargets.push_back(target);

#if ENABLE_OPENCV
    // also add the target to the _tracker
    _tracker->addARImageTarget(target);
#endif

    if (_imageTargets.size() > 0) {
        _haveActiveTargets = true;
    }

    if (_shouldTrack && _haveActiveTargets) {
        startTextureReader();
    }
}

void VROARTrackingSession::removeARImageTarget(std::shared_ptr<VROARImageTarget> target) {
    // remove the ARImageTarget from the list of tracked targets
    _imageTargets.erase(std::remove_if(_imageTargets.begin(), _imageTargets.end(),
                                  [target](std::shared_ptr<VROARImageTarget> candidate) {
                                      return candidate == target;
                                  }), _imageTargets.end());


    // If an anchor was found with the target, then notify the listener and remove it from the map.
    auto it = _targetAnchorMap.find(target);
    if (it != _targetAnchorMap.end()) {
        // notify the listener that the anchor was removed
        std::shared_ptr<VROARTrackingListener> listener = _weakListener.lock();
        if (listener) {
            listener->onTrackedAnchorRemoved(it->second);
        }

        // remove the target & anchor from the map
        _targetAnchorMap.erase(it);
    }

    // Add this target to the set of targets we need to remove (eventually)
    _targetsToRemove.insert(target);

    if (_imageTargets.size() == 0) {
        _haveActiveTargets = false;
        stopTextureReader();
    }
}

void VROARTrackingSession::startTextureReader() {
    if (!_isTextureReaderStarted && _frameSynchronizer && _cameraTextureReader) {
        _cameraTextureReader->start(_frameSynchronizer);
        _isTextureReaderStarted = true;
    }
}

void VROARTrackingSession::stopTextureReader() {
    if (_isTextureReaderStarted && _frameSynchronizer && _cameraTextureReader) {
        _cameraTextureReader->stop(_frameSynchronizer);
        _isTextureReaderStarted = false;
    }
}

void VROARTrackingSession::enableTracking(bool shouldTrack) {
    _shouldTrack = shouldTrack;
    if (_shouldTrack && _haveActiveTargets) {
        startTextureReader();
    } else {
        stopTextureReader();
    }
}

void VROARTrackingSession::flushTargetsToRemove() {
#if ENABLE_OPENCV
    if (!_tracker) {
        return;
    }

    auto it = _targetsToRemove.begin();
    for (; it != _targetsToRemove.end(); it++) {
        _tracker->removeARImageTarget(*it);
    }
#endif

    _targetsToRemove.clear();
}
