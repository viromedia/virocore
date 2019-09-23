//
//  VROPoseFilterMovingAverage.cpp
//  ViroKit
//
//  Created by Raj Advani on 2/26/19.
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

#include "VROPoseFilterMovingAverage.h"

VROPoseFrame VROPoseFilterMovingAverage::temporalFilter(const std::vector<VROPoseFrame> &frames, const VROPoseFrame &combinedFrame,
                                                        const VROPoseFrame &newFrame) {
    VROPoseFrame dampenedJoints = newPoseFrame();
    
    for (int i = 0; i < kNumBodyJoints; i++) {
        VROBodyJointType type = (VROBodyJointType) i;
        
        const std::vector<VROInferredBodyJoint> &samples = combinedFrame[i];
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
        
        VROInferredBodyJoint dampenedJoint(type);
        dampenedJoint.setCenter(averagePosition);
        dampenedJoint.setConfidence(averageConfidence);
        dampenedJoints[i] = { dampenedJoint };
    }
    
    return dampenedJoints;
}

