//
//  VROPoseFilterBoneDistance.h
//  ViroKit
//
//  Created by Raj Advani on 2/27/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

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
    
    void addJoints(const VROPoseFrame &joints, VROBodyJointType jointA, VROBodyJointType jointB,
                   VROPoseFrame *result);
    void addNonAnomalousJoints(const VROPoseFrame &frame, const VROPoseFrame &joints, VROBodyJointType jointA, VROBodyJointType jointB,
                               VROPoseFrame *result);
    VROBodyJointType getAnomalousJoint(const VROPoseFrame &frame, const VROPoseFrame &joints, VROBodyJointType jointA, VROBodyJointType jointB);
    float getDistanceFromPriors(const VROPoseFrame &frame, const VROPoseFrame &joints, VROBodyJointType joint);
    VROVector3f getPosition(const VROPoseFrame &frame, VROBodyJointType joint);
    
    float getAverageLimbLength(const std::vector<VROPoseFrame> &frames, VROBodyJointType jointA, VROBodyJointType jointB);
    float getLimbLength(const VROPoseFrame &frame, VROBodyJointType jointA, VROBodyJointType jointB);
    VROVector3f getLimbDirection(const VROPoseFrame &frame, VROBodyJointType jointA, VROBodyJointType jointB);
    VROVector3f getAverageLimbDirection(const std::vector<VROPoseFrame> &frames, VROBodyJointType jointA, VROBodyJointType jointB);
    
    
};

#endif /* VROPoseFilterBoneDistance_h */
