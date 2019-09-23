//
//  VROPoseFilterLowPass.cpp
//  ViroKit
//
//  Created by Raj Advani on 2/27/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
            
            VROVector3f emaToday = (sample.getCenter() * k) + (emaYesterday * (1 - k));
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
