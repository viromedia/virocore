//
//  VROPoseFilter.cpp
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

#include "VROPoseFilter.h"
#include "VROTime.h"
#include "VROLog.h"

static const float kConfidenceThreshold = 0.15;

float getCreationTime(const VROPoseFrame &frame) {
    for (int i = 0; i < kNumBodyJoints; i++) {
        if (!frame[i].empty()) {
            return frame[i][0].getCreationTime();
        }
    }
    return 0;
}

VROPoseFrame VROPoseFilter::filterJoints(const VROPoseFrame &frame) {
    VROPoseFrame newFrame = spatialFilter(_frames, _combinedFrame, frame);
    
    double windowEnd = VROTimeCurrentMillis();
    double windowStart = windowEnd - _trackingPeriodMs;
    
    // Remove old samples from the combined frame
    for (int i = 0; i < kNumBodyJoints; i++) {
        auto &joints = _combinedFrame[i];
        joints.erase(std::remove_if(joints.begin(), joints.end(),
                                    [windowStart](VROInferredBodyJoint &j) {
                                        return j.getCreationTime() < windowStart;
                                    }),
                     joints.end());
    }
    
    // Remove old frames
    _frames.erase(std::remove_if(_frames.begin(), _frames.end(),
                                 [windowStart](VROPoseFrame &f) {
                                     return getCreationTime(f) < windowStart;
                                 }),
                  _frames.end());
    
    // Update the frames vector, which contains all joints over time, organized
    // by frame
    _frames.push_back(newFrame);
    
    // Update the combined frame, which contains all joints over time, organized
    // by joint type
    for (int i = 0; i < kNumBodyJoints; i++) {
        auto &kv = frame[i];
        if (!kv.empty()) {
            if (kv[0].getConfidence() > kConfidenceThreshold) {
                _combinedFrame[i].push_back(kv[0]);
            }
        }
    }
    
    return temporalFilter(_frames, _combinedFrame, newFrame);
}
