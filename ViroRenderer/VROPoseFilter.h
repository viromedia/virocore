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

typedef std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> VROPoseFrame;

class VROPoseFilter {
public:
    
    /*
     Create a new pose filter that will operate on all received joints
     that are *above* the given confidence level. The filter will have
     acccess to all joint positions received over the past trackingPeriodMs.
     */
    VROPoseFilter(float trackingPeriodMs, float confidenceThreshold) :
        _trackingPeriodMs(trackingPeriodMs),
        _confidenceThreshold(confidenceThreshold) {}
    virtual ~VROPoseFilter() {}
    
    /*
     Invoked internally to filter a new set of joints. Returns the filtered
     set of joints.
     */
    VROPoseFrame filterJoints(const VROPoseFrame &frame);
    
protected:
    
    /*
     Return the joints we should add to the joint samples. This gives filters
     an opportunity to throw out joints entirely so they never enter the
     tracking window.
     
     The default behavior is not add all joints to the tracking window.
     */
    virtual VROPoseFrame processNewJoints(const std::vector<VROPoseFrame> &frames, const VROPoseFrame &combinedFrame,
                                          const VROPoseFrame &newFrame) {
        return newFrame;
    }
    
    /*
     Returns the filtered joints given the currently accumulated joints.
     */
    virtual VROPoseFrame doFilter(const std::vector<VROPoseFrame> &frames, const VROPoseFrame &combinedFrame,
                                  const VROPoseFrame &newFrame) = 0;
    
private:
    
    float _trackingPeriodMs;
    float _confidenceThreshold;
    std::vector<VROPoseFrame> _frames;
    VROPoseFrame _combinedFrame;
    
};

#endif /* VROPoseFilter_h */
