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
#include "VROMatrix4f.h"

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

class VROBodyJoint {
public:
    VROBodyJoint() : _type(VROBodyJointType::Top), _confidence(0) {}
    VROBodyJoint(VROBodyJointType type, VROVector3f screenCoords, double confidence) :
    _type(type),
    _screenCoords(screenCoords),
    _confidence(confidence),
    _projectedTransform(VROMatrix4f::identity()),
    _hasValidProjectedTransform(false){}

    const VROVector3f &getScreenCoords() const {
        return _screenCoords;
    }

    void setScreenCoords(VROVector3f screenCoords) {
        _screenCoords = screenCoords;
    }

    const VROMatrix4f &getProjectedTransform() const {
        return _projectedTransform;
    }

    void setProjectedTransform(VROMatrix4f transform) {
        _projectedTransform = transform;
        _hasValidProjectedTransform = true;
    }

    void clearPojectedTransform() {
        _projectedTransform.toIdentity();
        _hasValidProjectedTransform = false;
    }

    bool hasValidProjectedTransform() const {
        return _hasValidProjectedTransform;
    }

    void setSpawnTimeMs(double ts) {
        _spawnTimeMs = ts;
    }

    double getSpawnTimeMs() const {
        return _spawnTimeMs;
    }

    VROBodyJointType getType() const {
        return _type;
    }

    double getConfidence() const {
        return _confidence;
    }

private:
    VROVector3f _screenCoords;
    VROMatrix4f _projectedTransform;
    bool _hasValidProjectedTransform;
    VROBodyJointType _type;
    double _confidence;
    double _spawnTimeMs;
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
