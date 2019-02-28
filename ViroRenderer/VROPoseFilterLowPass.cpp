//
//  VROPoseFilterLowPass.cpp
//  ViroKit
//
//  Created by Raj Advani on 2/27/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROPoseFilterLowPass.h"

VROPoseFrame VROPoseFilterLowPass::temporalFilter(const std::vector<VROPoseFrame> &frames, const VROPoseFrame &combinedFrame,
                                                  const VROPoseFrame &newFrame) {
    VROPoseFrame dampenedJoints = newPoseFrame();
    
    for (int i = 0; i < kNumBodyJoints; i++) {
        VROBodyJointType type = (VROBodyJointType) i;
        
        const std::vector<VROInferredBodyJoint> &samples = combinedFrame[i];
        if (samples.empty()) {
            continue;
        }
        
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
        
        VROInferredBodyJoint dampenedJoint(type);
        dampenedJoint.setCenter(emaYesterday);
        dampenedJoint.setConfidence(sumConfidence / (float) samples.size());
        dampenedJoints[i] = { dampenedJoint };
    }
    
    return dampenedJoints;
}
