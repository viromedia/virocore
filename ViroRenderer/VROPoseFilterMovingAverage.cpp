//
//  VROPoseFilterMovingAverage.cpp
//  ViroKit
//
//  Created by Raj Advani on 2/26/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROPoseFilterMovingAverage.h"

JointMap VROPoseFilterMovingAverage::filterJoints(const JointWindow &jointWindow) {
    std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> dampenedJoints;
    
    for (auto &jointWindow : jointWindow) {
        const std::vector<std::pair<double, VROVector3f>> &posArray = jointWindow.second;
        
        VROVector3f net = VROVector3f();
        for (int i = 0; i < posArray.size(); i++) {
            net = net + posArray[i].second;
        }
        VROVector3f dampenedPoint = net / posArray.size();
        
        VROBodyJointType type = jointWindow.first;
        
        VROInferredBodyJoint dampenedJoint(type);
        dampenedJoint.setCenter(dampenedPoint);
        dampenedJoint.setConfidence(0.3);
        dampenedJoints[type] = { dampenedJoint };
    }
    
    return dampenedJoints;
}

