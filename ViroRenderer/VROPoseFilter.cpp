//
//  VROPoseFilter.cpp
//  ViroKit
//
//  Created by Raj Advani on 2/26/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROPoseFilter.h"
#include "VROTime.h"
#include "VROLog.h"

static const float kConfidenceThreshold = 0.15;

float getCreationTime(const VROPoseFrame &frame) {
    for (auto kv : frame) {
        if (!kv.second.empty()) {
            return kv.second[0].getCreationTime();
        }
    }
    return 0;
}

VROPoseFrame VROPoseFilter::filterJoints(const VROPoseFrame &frame) {
    VROPoseFrame newFrame = processNewJoints(_frames, _combinedFrame, frame);
    
    // Update the frames vector, which contains all joints over time, organized
    // by frame
    _frames.push_back(newFrame);
    
    // Update the combined frame, which contains all joints over time, organized
    // by joint type
    for (auto kv : newFrame) {
        if (!kv.second.empty()) {
            if (kv.second[0].getConfidence() > kConfidenceThreshold) {
                _combinedFrame[kv.first].push_back(kv.second[0]);
            }
        }
    }
    
    double windowEnd = VROTimeCurrentMillis();
    double windowStart = windowEnd - _trackingPeriodMs;
    
    // Remove old samples from the combined frame
    for (auto &kv : _combinedFrame) {
        auto &joints = kv.second;
        joints.erase(std::remove_if(joints.begin(), joints.end(),
                                    [windowStart](VROInferredBodyJoint &j) {
                                        return j.getCreationTime() < windowStart;
                                    }),
                     joints.end());
    }
    
    // Remove old frames
    _frames.erase(std::remove_if(_frames.begin(), _frames.end(),
                                [windowStart](VROPoseFrame &f) {
                                    return getCreationTime(f) < windowStart;
                                }),
                 _frames.end());
    return doFilter(_frames, _combinedFrame, newFrame);
}
