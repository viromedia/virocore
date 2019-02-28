//
//  VROPoseFilterLowPass.cpp
//  ViroKit
//
//  Created by Raj Advani on 2/27/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROPoseFilterLowPass.h"

VROPoseFrame VROPoseFilterLowPass::doFilter(const std::vector<VROPoseFrame> &frames, const VROPoseFrame &combinedFrame,
                                            const VROPoseFrame &newFrame) {
    std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> dampenedJoints;
    
    for (auto &type_samples : combinedFrame) {
        const std::vector<VROInferredBodyJoint> &samples = type_samples.second;
        
        float k = 2 / ((float) samples.size() + 1);
        VROVector3f emaYesterday = samples[0].getCenter();
        float sumConfidence = samples[0].getConfidence();
        
        // Exponentially weight towards the earliest data at the end of the array
        // (Items at the front of the array are older).
        for (int i = 1; i < samples.size(); i++) {
            const VROInferredBodyJoint &sample = samples[i];
            
            VROVector3f position = sample.getCenter();
            VROVector3f emaToday = (position * k) + (emaYesterday * (1 - k));
            emaYesterday = emaToday;
            
            sumConfidence += sample.getConfidence();
        }
        
        VROBodyJointType type = type_samples.first;
        
        VROInferredBodyJoint dampenedJoint(type);
        dampenedJoint.setCenter(emaYesterday);
        dampenedJoint.setConfidence(sumConfidence / (float) samples.size());
        dampenedJoints[type] = { dampenedJoint };
    }
    
    return dampenedJoints;
}
