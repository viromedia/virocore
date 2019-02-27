//
//  VROPoseFilter.h
//  ViroKit
//
//  Created by Raj Advani on 2/26/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROPoseFilter_h
#define VROPoseFilter_h

#include <map>
#include "VROBodyTracker.h"

typedef std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> JointMap;
typedef std::map<VROBodyJointType, std::vector<std::pair<double, VROVector3f>>> JointWindow;

class VROPoseFilter {
public:
    
    /*
     Create a new pose filter that will operate on all received joints
     that are *above* the given confidence level. The filter will have
     acccess to all joint positions received over the past trackingPeriodMs.
     */
    VROPoseFilter(float trackingPeriodMs, float confidenceThreshold) :
        _trackingPeriodMs(trackingPeriodMs) {}
    virtual ~VROPoseFilter() {}
    
    /*
     Invoked internally to filter a new set of joints. Returns the filtered
     set of joints.
     */
    JointMap filterJoints(const JointMap &joints);
    
protected:
    
    /*
     Implemented by subclasses, returns the filtered joints given the
     currently accumulated joints.
     */
    virtual JointMap filterJoints(const JointWindow &jointWindow) = 0;
    
private:
    
    float _trackingPeriodMs;
    float _confidenceThreshold;
    std::map<VROBodyJointType, std::vector<std::pair<double, VROVector3f>>> _jointWindow;
    
};

#endif /* VROPoseFilter_h */
