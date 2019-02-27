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

JointMap VROPoseFilter::filterJoints(const JointMap &joints) {
    JointMap jointsToAdd = processNewJoints(_jointWindow, joints);
    
    // Update the joint window dataset required for dampening
    for (auto kv : jointsToAdd) {
        if (!kv.second.empty()) {
            if (kv.second[0].getConfidence() > kConfidenceThreshold) {
                _jointWindow[kv.first].push_back(kv.second[0]);
            }
        }
    }
    
    // Update our window reference.
    double windowEnd = VROTimeCurrentMillis();
    double windowStart = windowEnd - _trackingPeriodMs;
    
    // Iterate through each joint type and remove joints outside the tracking
    // period
    for (auto &kv : _jointWindow) {
        auto &joints = kv.second;
        joints.erase(std::remove_if(joints.begin(), joints.end(),
                                      [windowStart](VROInferredBodyJoint &j) {
                                          return j.getCreationTime() < windowStart;
                                      }),
                     joints.end());
    }
    
    return doFilter(_jointWindow);
}
