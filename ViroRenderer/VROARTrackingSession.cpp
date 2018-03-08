//
//  VROARTrackingSession.cpp
//  ViroKit
//
// Created by Andy Chu on 3/7/18.
//  Copyright © 2017 Viro Media. All rights reserved.
//


#include "VROARTrackingSession.h"
#include <algorithm>

void VROARTrackingSession::updateFrame(std::unique_ptr<VROARFrame> frame) {

}

void VROARTrackingSession::setListener(std::shared_ptr<VROARTrackingListener> listener) {
    _weakListener = listener;
}

void VROARTrackingSession::addARImageTarget(std::shared_ptr<VROARImageTarget> target) {
    _imageTargets.push_back(target);

    _targetAnchorMap[target] = std::make_shared<VROARImageTarget>(target);
}

void VROARTrackingSession::removeARImageTarget(std::shared_ptr<VROARImageTarget> target) {
    // add target to the list of tracked targets
    _imageTargets.erase(std::remove_if(_imageTargets.begin(), _imageTargets.end(),
                                  [target](std::shared_ptr<VROARImageTarget> candidate) {
                                      return candidate == target;
                                  }), _imageTargets.end());

    // notify the listener that the anchor was removed
    std::shared_ptr<VROARTrackingListener> listener = _weakListener.lock();
    if (listener) {
        listener->onARAnchorRemoved(_targetAnchorMap[target]);
    }

    // remove the target & anchor from the map
    auto it = _targetAnchorMap.find(target);
    _targetAnchorMap.erase(it);
}