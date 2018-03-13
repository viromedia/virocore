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
#include <algorithm>

#if VRO_PLATFORM_ANDROID
#include "VROARImageTargetAndroid.h"
#endif

VROARTrackingSession::VROARTrackingSession() {
#if ENABLE_OPENCV
    _tracker = VROARImageTracker::createDefaultTracker();
#endif // ENABLE_OPENCV
}

VROARTrackingSession::~VROARTrackingSession() {
    
}

void VROARTrackingSession::updateFrame(VROARFrame *frame) {
    if (_imageTargets.size() == 0) {
        return;
    }

    bool shouldMock = false;
    // TODO: below code is for mocking this class
    if (shouldMock) {
        mockTracking();
    } else {
#if ENABLE_OPENCV

        // TODO: also check for a "isReady" flag.
        // TODO: convert frame to cv::Mat
        //     see ImageTracker_JNI.cpp for Android Bitmap -> cv::Mat (parseBitmapImage)
        //     see VROTrackingHelper.mm for iOS pixelbuffer -> cv::Mat (matFromYCbCrBuffer)
        cv::Mat matImage;

        // invoke VROARImageTracker.findTarget on a background thread.
        // TODO: grab the intrinsic matrix...
        VROPlatformDispatchAsyncBackground([this, matImage] {
            std::vector<std::shared_ptr<VROARImageTrackerOutput>> outputs = _tracker->findTarget(matImage, NULL);
            for (int i = 0; i < outputs.size(); i++) {
                std::shared_ptr<VROARImageTrackerOutput> output = outputs[i];
                std::shared_ptr<VROARTrackingListener> listener = _weakListener.lock();

                if (output->found && listener) {
                    auto it = _targetAnchorMap.find(output->target);
                    if (it != _targetAnchorMap.end()) {
                        // there's already an anchor, so notify that it's been updated.
                        std::shared_ptr<VROARAnchor> anchor = it->second;
                        // TODO: update the anchor's transform with the output transform!
                        
                        // TODO: should this call back on the render thread?
                        listener->onTrackedAnchorUpdated(anchor);
                    } else {
                        // there hasn't been an anchor created, so create an anchor and notify that it has been found
                        // TODO: you also need to set the ID on the VROARImageAnchor for Android (methinks), iOS uses
                        // the attached anchors to determine if an anchor is for a target (why can't we do that?)
                        std::shared_ptr<VROARImageAnchor> imageAnchor = std::make_shared<VROARImageAnchor>(output->target);
                        // TODO: update anchor's transform with the output transform!
                        _targetAnchorMap[output->target] = imageAnchor;
                        
                        // VROARImageAnchor is a type of VROARAnchor, but Xcode complains if I don't cast.
                        output->target->setAnchor(std::dynamic_pointer_cast<VROARAnchor>(imageAnchor));

                        // TODO: should this call back on the render thread?
                        listener->onTrackedAnchorFound(std::dynamic_pointer_cast<VROARAnchor>(imageAnchor));
                    }
                }
            }
        });

#endif // ENABLE_OPENCV
    }
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
}

void VROARTrackingSession::mockTracking() {
#if VRO_PLATFORM_ANDROID
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
#endif // VRO_PLATFORM_ANDROID
}
