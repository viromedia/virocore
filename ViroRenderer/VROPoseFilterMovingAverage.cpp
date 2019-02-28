//
//  VROPoseFilterMovingAverage.cpp
//  ViroKit
//
//  Created by Raj Advani on 2/26/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROPoseFilterMovingAverage.h"

VROPoseFrame VROPoseFilterMovingAverage::temporalFilter(const std::vector<VROPoseFrame> &frames, const VROPoseFrame &combinedFrame,
                                                        const VROPoseFrame &newFrame) {
    std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> dampenedJoints;
    
    for (auto &type_samples : combinedFrame) {
        const std::vector<VROInferredBodyJoint> &samples = type_samples.second;
        if (samples.empty()) {
            continue;
        }
        
        VROVector3f sumPosition;
        float sumConfidence = 0;
        for (const VROInferredBodyJoint &sample : samples) {
            sumPosition += sample.getCenter();
            sumConfidence += sample.getConfidence();
        }
        VROVector3f averagePosition = sumPosition / (float) samples.size();
        float averageConfidence = sumConfidence / (float) samples.size();
        
        VROBodyJointType type = type_samples.first;
        
        VROInferredBodyJoint dampenedJoint(type);
        dampenedJoint.setCenter(averagePosition);
        dampenedJoint.setConfidence(averageConfidence);
        dampenedJoints[type] = { dampenedJoint };
    }
    
    return dampenedJoints;
}

