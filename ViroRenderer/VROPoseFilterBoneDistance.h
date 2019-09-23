//
//  VROPoseFilterBoneDistance.h
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

#ifndef VROPoseFilterBoneDistance_h
#define VROPoseFilterBoneDistance_h

#include "VROPoseFilter.h"

/*
 Filter that removes joints that are more than a given percentage away
 from their parent than the average limb length.
 
 For example, if the average distance between leg limbs (hip to knee, knee
 to ankle) is 5, and we receive a knee that is 15 away from the hip and
 15 away from the ankle, we'll discard that knee joint.
 */
class VROPoseFilterBoneDistance : public VROPoseFilter {
public:
    
    VROPoseFilterBoneDistance(float trackingPeriodMs, float confidenceThreshold) :
        VROPoseFilter(trackingPeriodMs, confidenceThreshold) {}
    virtual ~VROPoseFilterBoneDistance() {}
    
    VROPoseFrame spatialFilter(const std::vector<VROPoseFrame> &frames, const VROPoseFrame &combinedFrame,
                               const VROPoseFrame &newFrame);
    VROPoseFrame temporalFilter(const std::vector<VROPoseFrame> &frames, const VROPoseFrame &combinedFrame,
                                const VROPoseFrame &newFrame);

private:
    
    VROVector3f getJointPosition(const VROPoseFrame &frame, VROBodyJointType joint);
    float getAverageLimbLength(const std::vector<VROPoseFrame> &frames, VROBodyJointType jointA, VROBodyJointType jointB);
    float getLimbLength(const VROPoseFrame &frame, VROBodyJointType jointA, VROBodyJointType jointB);
    VROVector3f getLimbDirection(const VROPoseFrame &frame, VROBodyJointType jointA, VROBodyJointType jointB);
    VROVector3f getAverageLimbDirection(const std::vector<VROPoseFrame> &frames, VROBodyJointType jointA, VROBodyJointType jointB);
    
};

#endif /* VROPoseFilterBoneDistance_h */
