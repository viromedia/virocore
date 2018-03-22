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
}

VROARTrackingSession::~VROARTrackingSession() {
    
}

void VROARTrackingSession::createTextureReader(VROARFrameARCore *frame,
                                               GLuint cameraTextureId,
                                               int width, int height) {
    VROVector3f BL, BR, TL, TR;
    frame->getBackgroundTexcoords(&BL, &BR, &TL, &TR);

    _cameraTextureReader = std::make_shared<VROTextureReader>(cameraTextureId,
                                                              width, height,
                                                              width, height,
                                                              1, // check every frame if we're ready
                                                              VROTextureReaderOutputFormat::RGBA8,
            [width, height, this](std::shared_ptr<VROData> data) {
                if (!_readyForTracking || !_haveActiveTargets) {
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
                    // TODO: get intrinsics matrix.
                    std::vector<VROARImageTrackerOutput> outputs = _tracker->findTarget(tmpFrame, NULL, _lastCamera);

                    for (int i = 0; i < outputs.size(); i++) {
                        VROARImageTrackerOutput output = outputs[i];
                        if (!output.found) {
                            continue;
                        }

                        bool isUpdate;
                        std::shared_ptr<VROARImageAnchor> imageAnchor;
                        auto it = _targetAnchorMap.find(output.target);
                        if (it == _targetAnchorMap.end()) {
                            // call the _listener.onAnchorFound w/ the anchor for output.target
                            imageAnchor = std::make_shared<VROARImageAnchor>(output.target);
                            _targetAnchorMap[output.target] = imageAnchor;
                            isUpdate = false;
                        } else {
                            // call the _listener.onAnchorUpdated w/ the anchor for output.target
                            imageAnchor = it->second;
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
                    _readyForTracking = true;
                });
            });

    _cameraTextureReader->setTextureCoordinates(BL, BR, TL, TR);
    _cameraTextureReader->init();
}

void VROARTrackingSession::startTextureReader(std::shared_ptr<VROFrameSynchronizer> synchronizer) {
    _cameraTextureReader->start(synchronizer);
}

void VROARTrackingSession::updateFrame(VROARFrame *frame) {
    if (_imageTargets.size() == 0) {
        return;
    }

    // TODO: we're storing this camera, hopefully it's in sync with the callback we get from
    // the texture reader...
    _lastCamera = frame->getCamera();

    bool shouldMock = false;
    // TODO: below code is for mocking this class
    if (shouldMock) {
        mockTracking();
    } else {
        return;
    }
}

void VROARTrackingSession::setListener(std::shared_ptr<VROARTrackingListener> listener) {
    _weakListener = listener;
}

void VROARTrackingSession::addARImageTarget(std::shared_ptr<VROARImageTarget> target) {
    _imageTargets.push_back(target);

    if (_imageTargets.size() > 0) {
        _haveActiveTargets = true;
    }

#if ENABLE_OPENCV
    // also add the target to the _tracker
    _tracker->addARImageTarget(target);
#endif
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

#if ENABLE_OPENCV
    // also remove the target from the _tracker
    _tracker->removeARImageTarget(target);
#endif

    if (_imageTargets.size() == 0) {
        _haveActiveTargets = false;
    }
}

void VROARTrackingSession::mockTracking() {
    _count++;
    if (_count == 180) { // 3 seconds at 60 fps, 6 seconds at 30 fps
        std::shared_ptr<VROARTrackingListener> listener = _weakListener.lock();
        if (listener) {
            pinfo("VROARTrackingSession - mock found");
            // create a new ImageAnchor!
            _imageAnchor = std::make_shared<VROARImageAnchor>(_imageTargets[0]);
            std::shared_ptr<VROARImageTargetAndroid> imageTargetAndroid =
            std::dynamic_pointer_cast<VROARImageTargetAndroid>(_imageTargets[0]);
            _imageAnchor->setId(imageTargetAndroid->getId());
            
            VROMatrix4f mat;
            mat.rotateZ(-90); // rotate clockwise 90 WRT to initial camera position
            mat.translate({0, 0, -.5}); // start 1/2 a meter from the intial camera position
            
            _imageAnchor->setTransform(mat);
            
            listener->onTrackedAnchorFound(_imageAnchor);
        }
    } else if(_count > 180 && _count % 60 == 0 && _count < 600) { // every 60 frames after that...
        std::shared_ptr<VROARTrackingListener> listener = _weakListener.lock();
        if (listener) {
            pinfo("VROARTrackingSession - mock updated");
            // update the ImageAnchor!
            VROMatrix4f mat = _imageAnchor->getTransform();
            mat.translate({0,0,-.15f});
            _imageAnchor->setTransform(mat);
            listener->onTrackedAnchorUpdated(_imageAnchor);
        }
    } else if (_count == 600) {
        std::shared_ptr<VROARTrackingListener> listener = _weakListener.lock();
        if (listener) {
            pinfo("VROARTrackingSession - mock removed");
            // remove the ImageAnchor!
            listener->onTrackedAnchorRemoved(_imageAnchor);
        }
    }
}
