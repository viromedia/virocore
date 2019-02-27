//
//  VROPoseFilterMovingAverage.cpp
//  ViroKit
//
//  Created by Raj Advani on 2/26/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROPoseFilterMovingAverage.h"

JointMap VROPoseFilterMovingAverage::doFilter(const JointMap &jointWindow) {
    std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> dampenedJoints;
    
    for (auto &type_joint : jointWindow) {
        const std::vector<VROInferredBodyJoint> &joints = type_joint.second;
        
        VROVector3f sumPosition;
        float sumConfidence = 0;
        for (const VROInferredBodyJoint &joint : joints) {
            sumPosition += joint.getCenter();
            sumConfidence += joint.getConfidence();
        }
        VROVector3f averagePosition = sumPosition / (float) joints.size();
        float averageConfidence = sumConfidence / (float) joints.size();
        
        VROBodyJointType type = type_joint.first;
        
        VROInferredBodyJoint dampenedJoint(type);
        dampenedJoint.setCenter(averagePosition);
        dampenedJoint.setConfidence(averageConfidence);
        dampenedJoints[type] = { dampenedJoint };
    }
    
    return dampenedJoints;
}

