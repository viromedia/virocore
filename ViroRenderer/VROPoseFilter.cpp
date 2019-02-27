//
//  VROPoseFilter.cpp
//  ViroKit
//
//  Created by Raj Advani on 2/26/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROPoseFilter.h"
#include "VROTime.h"

static const float kConfidenceThreshold = 0.15;

JointMap VROPoseFilter::filterJoints(const JointMap &joints) {
    // Update the joint window dataset required for dampening
    for (auto kv : joints) {
        if (!kv.second.empty()) {
            if (kv.second[0].getConfidence() > kConfidenceThreshold) {
                VROVector3f position = kv.second[0].getBounds().getCenter();
                std::pair<double, VROVector3f> p = std::make_pair(VROTimeCurrentMillis(), position);
                _jointWindow[kv.first].push_back(p);
            }
        }
    }
    
    // Update our window reference.
    double windowEnd = VROTimeCurrentMillis();
    double windowStart = windowEnd - _trackingPeriodMs;
    
    // Iterate through each joint type and remove joints outside the tracking
    // period
    for (auto &kv : _jointWindow) {
        std::vector<std::pair<double, VROVector3f>> &positions = kv.second;
        positions.erase(std::remove_if(positions.begin(), positions.end(),
                                      [windowStart](std::pair<double, VROVector3f> p) {
                                          return p.first < windowStart;
                                      }),
                       positions.end());
        
    }
    
    return filterJoints(_jointWindow);
}
