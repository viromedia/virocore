//
//  VROPoseFilterLowPass.cpp
//  ViroKit
//
//  Created by Raj Advani on 2/27/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROPoseFilterLowPass.h"

JointMap VROPoseFilterLowPass::doFilter(const JointMap &jointWindow) {
    std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> dampenedJoints;
    
    for (auto &type_joint : jointWindow) {
        const std::vector<VROInferredBodyJoint> &joints = type_joint.second;
        
        float k = 2 / ((float) joints.size() + 1);
        VROVector3f emaYesterday = joints[0].getCenter();
        float sumConfidence = joints[0].getConfidence();
        
        // Exponentially weight towards the earliest data at the end of the array
        // (Items at the front of the array are older).
        for (int i = 1; i < joints.size(); i++) {
            const VROInferredBodyJoint &joint = joints[i];
            
            VROVector3f position = joint.getCenter();
            VROVector3f emaToday = (position * k) + (emaYesterday * (1 - k));
            emaYesterday = emaToday;
            
            sumConfidence += joint.getConfidence();
        }
        
        VROBodyJointType type = type_joint.first;
        
        VROInferredBodyJoint dampenedJoint(type);
        dampenedJoint.setCenter(emaYesterday);
        dampenedJoint.setConfidence(sumConfidence / (float) joints.size());
        dampenedJoints[type] = { dampenedJoint };
    }
    
    return dampenedJoints;
}
