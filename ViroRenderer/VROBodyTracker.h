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
#include "VROVisionModel.h"
#include "VROVector3f.h"
#include "VROMatrix4f.h"
#include "VROBoundingBox.h"

// Known body joint types recognized by VROBodyTracker
// Note: Enum values matter as they are being utilized in VROBodyTracker!
enum class VROBodyJointType {
    Top                 = 0,
    Neck                = 1,
    RightShoulder       = 2,
    RightElbow          = 3,
    RightWrist          = 4,
    RightHip            = 8,
    RightKnee           = 9,
    RightAnkle          = 10,
    LeftShoulder        = 5,
    LeftElbow           = 6,
    LeftWrist           = 7,
    LeftHip             = 11,
    LeftKnee            = 12,
    LeftAnkle           = 13
};

class VROInferredBodyJoint {
public:
    
    VROInferredBodyJoint() : _confidence(0) {}
    VROInferredBodyJoint(VROBodyJointType type, VROBoundingBox bounds, double confidence) :
        _type(type),
        _bounds(bounds),
        _confidence(confidence) {}
    
    const VROBoundingBox &getBounds() const {
        return _bounds;
    }
    void setBounds(VROBoundingBox bounds) {
        _bounds = bounds;
    }
    
    VROBodyJointType getType() const {
        return _type;
    }
    double getConfidence() const {
        return _confidence;
    }
    
private:
    VROBoundingBox _bounds;
    VROBodyJointType _type;
    double _confidence;
    double _spawnTimeMs;
};

class VROBodyTrackerDelegate {
public:
    virtual void onBodyJointsFound(const std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> &joints) = 0;
};

class VROBodyTracker : public VROVisionModel {
    
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
