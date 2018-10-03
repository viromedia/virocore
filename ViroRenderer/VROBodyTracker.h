//
//  VROBodyTracker.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/3/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROBodyTracker_h
#define VROBodyTracker_h

#include <memory>
#include <map>
#include "VROVector3f.h"

enum class VROBodyJointType {
    Top = 0,
    Neck = 1,
    RightShoulder = 2,
    RightElbow = 3,
    RightWrist = 4,
    LeftShoulder = 5,
    LeftElbow = 6,
    LeftWrist = 7,
    RightHip = 8,
    RightKnee = 9,
    RightAnkle = 10,
    LeftHip = 11,
    LeftKnee = 12,
    LeftAnkle = 13,
};

class VROBodyJoint {
public:
    VROBodyJoint() : _type(VROBodyJointType::Top), _confidence(0) {}
    VROBodyJoint(VROBodyJointType type, VROVector3f point, double confidence) :
    _type(type), _point(point), _confidence(confidence) {
    }
    
    const VROVector3f &getPoint() const { return _point; }
    void setPoint(VROVector3f point) { _point = point; }
    VROBodyJointType getType() const { return _type; }
    double getConfidence() const { return _confidence; }
    
private:
    VROBodyJointType _type;
    VROVector3f _point;
    double _confidence;
};

class VROBodyTrackerDelegate {
public:
    virtual void onBodyJointsFound(const std::map<VROBodyJointType, VROBodyJoint> &joints) = 0;
};

class VROBodyTracker {
    
public:
    VROBodyTracker() {};
    virtual ~VROBodyTracker() {}
    
    virtual void startBodyTracking() = 0;
    virtual void stopBodyTracking() = 0;
    
    void setDelegate(std::shared_ptr<VROBodyTrackerDelegate> delegate) {
        _bodyMeshDelegate_w = delegate;
    }
    
protected:
    std::weak_ptr<VROBodyTrackerDelegate> _bodyMeshDelegate_w;
    
};

#endif /* VROBodyTracker_h */
