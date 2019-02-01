//
//  VROSkeleton.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROSkeleton_h
#define VROSkeleton_h

class VROBone;
class VRONode;

#include <memory>
#include <vector>
#include <map>
#include <string>
#include "VROMatrix4f.h"

/*
 The VROSkeleton is the bone-structure that defines the control points
 for a skeletal animation. These control points (the bones) are the animatable
 objects that drive skeletal animations.
 
 To associate a skeleton with a geometry, use a VROSkinner.
 */
class VROSkeleton {
    
public:
    
    VROSkeleton(std::vector<std::shared_ptr<VROBone>> bones);
    virtual ~VROSkeleton() {}
    
    int getNumBones() const {
        return (int)_bones.size();
    }

    const std::shared_ptr<VROBone> getBone(int index) const {
        return _bones[index];
    }

    const std::shared_ptr<VROBone> getBone(std::string name) const {
        auto bone = _nameToBonesMap.find(name);
        if (bone != _nameToBonesMap.end()) {
            return bone->second;
        }
        return nullptr;
    }

    /*
     Returns the world transform of the given bone.
     TODO VIRO-4901: Implement propert bone binding transforms for gLTF models as well
     */
    VROMatrix4f getCurrentBoneWorldTransform(int boneId);
    VROMatrix4f getCurrentBoneWorldTransform(std::string boneName);
    VROMatrix4f getCurrentBoneWorldTransform(std::shared_ptr<VROBone> bone);

    /*
     Sets the world transform of the given bone.
     TODO VIRO-4901: Implement propert bone binding transforms for gLTF models as well
     */
    void setCurrentBoneWorldTransform(int boneId, VROMatrix4f transform, bool recurse);
    void setCurrentBoneWorldTransform(std::string boneName, VROMatrix4f transform, bool recurse);
    void setCurrentBoneWorldTransform(std::shared_ptr<VROBone> bone, VROMatrix4f transform, bool recurse);

    /*
     Sets a weak reference to the model's root for boneToWorld calculations.
     */
    void setModelRootNode(std::shared_ptr<VRONode> modelRootNode);

    /*
     Returns the model's root node referenced by this VROSkeleton.
     */
    std::shared_ptr<VRONode> getModelRootNode();
private:
    
    /*
     The bones representing the skeleton. Each represents a control point of
     the animation. Moving a bone deforms the surface of this skinner's
     geometry.
     */
    std::vector<std::shared_ptr<VROBone>> _bones;

    /*
     A map of all known bone names to bones
     */
    std::map<std::string, std::shared_ptr<VROBone>> _nameToBonesMap;

    /*
     Model root reference for boneToWorld calculations.
     */
    std::weak_ptr<VRONode> _modelRootNode_w;
};

#endif /* VROSkeleton_h */
