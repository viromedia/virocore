//
//  VROBodyPlayeriOS.cpp
//  ViroRenderer
//
//  Created by Vik Advani on 1/21/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//
#import <Foundation/Foundation.h>
#include "VROBodyPlayeriOS.h"
#include "VRORenderContext.h"

// constants that represent JSON keys from JSON body animation.
const std::string kBodyAnimTotalTime = "totalTime";
const std::string kBodyAnimAnimRows = "animRows";
const std::string kBodyAnimJoints = "joints";
const std::string kBodyAnimTimestamp = "timestamp";
const std::string kBodyAnimInitModelTransform = "initModelTransform";
const std::string kBodyAnimVersion = "version";

VROBodyPlayeriOS::VROBodyPlayeriOS() {
    _playbackInfo = nil;
}

void VROBodyPlayeriOS::start() {
    if (_playbackInfo) {
        _playbackInfo->start();
    }
}

void VROBodyPlayeriOS::pause() {
    if (_playbackInfo) {
        _playbackInfo->pause();
    }
}

void VROBodyPlayeriOS::setTime(double time) {
    if (_playbackInfo) {
        _playbackInfo->setTime(time);
    }
}

void VROBodyPlayeriOS::onFrameWillRender(const VRORenderContext &context) {
    if (_playbackInfo == NULL) {
        return;
    }

    if (_playbackInfo->isFinished()) {
        return;
    }

    if (_playbackInfo->getPlayStatus() == VROBodyPlayerStatus::Paused) {
        return;
    }

    double currentTime = VROTimeCurrentMillis();
    double frameTime = currentTime - _playbackInfo->getStartTime();
    double currentRowTime = _playbackInfo->getCurrentRowTimestamp();
     std::shared_ptr<VROBodyPlayerDelegate> bodyPlayerDelegate = _bodyMeshDelegate_w.lock();
    if (_playbackInfo->getPlayStatus() == VROBodyPlayerStatus::Start) {
         if (bodyPlayerDelegate != NULL) {
             VROMatrix4f matrix = _playbackInfo->getInitWorldMatrix();
             bodyPlayerDelegate->onBodyPlaybackStarting(matrix);
         }
    }

    // If frameTime is >= time in current animation row, then send the body joint data to the delegate so it can be animated.
    if (frameTime >= currentRowTime) {
        std::map<VROBodyJointType, VROVector3f> jointMap = _playbackInfo->getCurrentRowJointsAsMap();
        if (bodyPlayerDelegate != NULL) {
            bodyPlayerDelegate->onBodyJointsPlayback(jointMap, _playbackInfo->getPlayStatus());
        }
        _playbackInfo->incrementAnimRow();
    }
}

void VROBodyPlayeriOS::prepareAnimation(std::shared_ptr<VROBodyAnimData> bodyAnimData) {
    _playbackInfo = std::make_shared<BodyPlaybackInfo>(bodyAnimData);
}

void VROBodyPlayeriOS::onFrameDidRender(const VRORenderContext &context) {
    // no-op
}

