//
//  VROARTrackingSession.cpp
//  ViroKit
//
// Created by Andy Chu on 3/7/18.
//  Copyright © 2017 Viro Media. All rights reserved.
//


#include "VROARTrackingSession.h"
#include "VROARImageAnchor.h"
#include "VROARImageTargetAndroid.h"
#include <algorithm>

void VROARTrackingSession::updateFrame(VROARFrame *frame) {
    // TODO: below code is for mocking this class
    if (_imageTargets.size() == 0) {
        return;
    }

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

void VROARTrackingSession::setListener(std::shared_ptr<VROARTrackingListener> listener) {
    _weakListener = listener;
}

void VROARTrackingSession::addARImageTarget(std::shared_ptr<VROARImageTarget> target) {
    _imageTargets.push_back(target);

    // TODO: you also need to set the ID on the VROARImageAnchor for Android (methinks)
    _targetAnchorMap[target] = std::make_shared<VROARImageAnchor>(target);
}

void VROARTrackingSession::removeARImageTarget(std::shared_ptr<VROARImageTarget> target) {

    // remove the ARImageTarget from the list of tracked targets
    _imageTargets.erase(std::remove_if(_imageTargets.begin(), _imageTargets.end(),
                                  [target](std::shared_ptr<VROARImageTarget> candidate) {
                                      return candidate == target;
                                  }), _imageTargets.end());


    // If an anchor was found with the target, then notify the listener and remove it from the map.
    // TODO: just a thought, if we do this right, we don't need _targetAnchorMap b/c target has a
    // ptr to anchor
    auto it = _targetAnchorMap.find(target);
    if (it != _targetAnchorMap.end()) {
        // notify the listener that the anchor was removed
        std::shared_ptr<VROARTrackingListener> listener = _weakListener.lock();
        if (listener) {
            listener->onTrackedAnchorRemoved(_targetAnchorMap.find(target)->second);
        }

        // remove the target & anchor from the map
        _targetAnchorMap.erase(it);
    }
}