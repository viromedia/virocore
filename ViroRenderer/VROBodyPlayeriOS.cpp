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

VROBodyPlayeriOS::VROBodyPlayeriOS() {
    _playbackInfo = nil;
}

void VROBodyPlayeriOS::prepareAnimation(std::string animData) {
    NSString *animJSString = [NSString stringWithCString:animData.c_str()
                                              encoding:[NSString defaultCStringEncoding]];
    NSError *error = nil;
    NSData *data = [animJSString dataUsingEncoding:NSUTF8StringEncoding];
    NSDictionary *animDictionary = [NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:&error];
    _playbackInfo = new BodyPlaybackInfo(animDictionary);
}

void VROBodyPlayeriOS::start() {
    if (_playbackInfo) {
        _playbackInfo->startPlayback();
    }
}

void VROBodyPlayeriOS::stop() {

}

void VROBodyPlayeriOS::setTime() {

}

void VROBodyPlayeriOS::onFrameWillRender(const VRORenderContext &context) {
    if (_playbackInfo == NULL) {
        return;
    }

    if (_playbackInfo->isFinished()) {
        return;
    }

    double currentTime  = VROTimeCurrentMillis();
    double frameTime = currentTime - _playbackInfo->getStartTime();
    double currentRowTime = _playbackInfo->getCurrentRowTimestamp();
     std::shared_ptr<VROBodyPlayerDelegate> bodyPlayerDelegate = _bodyMeshDelegate_w.lock();
    if (_playbackInfo->getPlayStatus() == VROBodyPlayerStatus::Start) {
         if (bodyPlayerDelegate != NULL) {
             float array[16];
             int i=0;

             NSArray *nsArray = _playbackInfo->getMatrixStartArray();
             for (NSNumber *number in nsArray) {
                 array[i] = [number floatValue];
                 i++;
             }

             VROMatrix4f matrix(array);
             bodyPlayerDelegate->onBodyPlaybackStarting(matrix);
         }
    }
    // If frameTime is >= time in current animation row, then send the body joint data to the delegate so it can be animated.
    if (frameTime >= currentRowTime) {
        std::map<VROBodyJointType, VROVector3f> jointMap = _playbackInfo->getCurrentRowJointsAsMap();
        //std::shared_ptr<VROBodyPlayerDelegate> bodyPlayerDelegate = _bodyMeshDelegate_w.lock();
        if(bodyPlayerDelegate != NULL) {
            bodyPlayerDelegate->onBodyJointsPlayback(jointMap, _playbackInfo->getPlayStatus());
        }
        _playbackInfo->incrementAnimRow();
    }
}

void VROBodyPlayeriOS::onFrameDidRender(const VRORenderContext &context) {
    // no-op
}

