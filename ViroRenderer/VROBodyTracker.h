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

class VRODriver;
enum class VROCameraPosition;

static const int kNumBodyJoints = 17;

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
    LeftAnkle           = 13,
    Thorax              = 14,
    Pelvis              = 15,
    Unknown             = 16
};

// Supported bone names within 3D models for ml joint tracking.
static const std::map<VROBodyJointType, std::string> kVROBodyBoneTags = {
    {VROBodyJointType::Top,             "Top"},
    {VROBodyJointType::Neck,            "Neck"},
    {VROBodyJointType::RightShoulder,   "RightShoulder"},
    {VROBodyJointType::RightElbow,      "RightElbow"},
    {VROBodyJointType::RightWrist,      "RightWrist"},
    {VROBodyJointType::RightHip,        "RightHip"},
    {VROBodyJointType::RightKnee,       "RightKnee"},
    {VROBodyJointType::RightAnkle,      "RightAnkle"},
    {VROBodyJointType::LeftShoulder,    "LeftShoulder"},
    {VROBodyJointType::LeftElbow,       "LeftElbow"},
    {VROBodyJointType::LeftWrist,       "LeftWrist"},
    {VROBodyJointType::LeftHip,         "LeftHip"},
    {VROBodyJointType::LeftKnee,        "LeftKnee"},
    {VROBodyJointType::LeftAnkle,       "LeftAnkle"},
};


class VROInferredBodyJoint {
public:
    
    VROInferredBodyJoint() : _confidence(0) {}
    VROInferredBodyJoint(VROBodyJointType type) :
        _type(type) {}
    
    const VROBoundingBox &getBounds() const {
        return _bounds;
    }
    void setBounds(VROBoundingBox bounds) {
        _bounds = bounds;
    }
    VROVector3f getCenter() const {
        return _bounds.getCenter();
    }
    void setCenter(VROVector3f center) {
        _bounds = VROBoundingBox(center.x, center.x, center.y, center.y, 0, 0);
    }
    
    VROBodyJointType getType() const {
        return _type;
    }
    
    double getConfidence() const {
        return _confidence;
    }
    void setConfidence(float confidence) {
        _confidence = confidence;
    }
    
    void setTileIndices(int x, int y) {
        _tileX = x;
        _tileY = y;
    }
    int getTileX() const { return _tileX; }
    int getTileY() const { return _tileY; }
    
private:
    VROBoundingBox _bounds;
    VROBodyJointType _type;
    double _confidence;
    double _spawnTimeMs;
    int _tileX, _tileY;
};

class VROBodyTrackerDelegate {
public:
    virtual void onBodyJointsFound(const std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> &joints) = 0;
};

class VROBodyTracker : public VROVisionModel {
    
public:
    VROBodyTracker() {};
    virtual ~VROBodyTracker() {}
    
    virtual bool initBodyTracking(VROCameraPosition position, std::shared_ptr<VRODriver> driver) = 0;
    virtual void startBodyTracking() = 0;
    virtual void stopBodyTracking() = 0;
    
    void setDelegate(std::shared_ptr<VROBodyTrackerDelegate> delegate) {
        _bodyMeshDelegate_w = delegate;
    }
    
protected:
    std::weak_ptr<VROBodyTrackerDelegate> _bodyMeshDelegate_w;
    
};

#endif /* VROBodyTracker_h */
