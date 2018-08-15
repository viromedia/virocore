//
//  VROSkinner.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSkinner.h"
#include "VROSkeleton.h"
#include "VROBone.h"
#include "VROMatrix4f.h"
#include "VROMath.h"

VROSkinner::VROSkinner(std::shared_ptr<VROSkeleton> skeleton,
                       VROMatrix4f geometryBindTransform,
                       std::vector<VROMatrix4f> boneSpaceTransforms,
                       std::shared_ptr<VROGeometrySource> boneIndices,
                       std::shared_ptr<VROGeometrySource> boneWeights) :
    _skeleton(skeleton),
    _boneIndices(boneIndices),
    _boneWeights(boneWeights) {
        
    /*
     We are given geometryBindTransform, which moves the model to the bind pose, and
     the boneSpaceTransforms, which for each bone move from model space
     to said bone's own local space. Use these to construct the _bindTransforms and
     _inverseBindTransforms for each bone. These matrices go to and from the encoded
     position in model space to the bind position in bone local space.
     */
    for (VROMatrix4f &boneSpaceTransform : boneSpaceTransforms) {
        VROMatrix4f bindTransform = boneSpaceTransform.multiply(geometryBindTransform);
        _bindTransforms.push_back(bindTransform);
        _inverseBindTransforms.push_back(bindTransform.invert());
    }
}

VROMatrix4f VROSkinner::getModelTransform(int boneIndex) {
    const std::shared_ptr<VROBone> &bone = _skeleton->getBone(boneIndex);

    /*
     Model space, original position --> [bindTransform]                 --> Bone space, bind position
     Bone space, bind position      --> [boneTransform]                 --> Bone space, animated position
     Bone space, animated position  --> [inverseBindTransform]          --> Model space, animated position
     */
    if (bone->getTransformType() == VROBoneTransformType::Legacy) {
        return _inverseBindTransforms[boneIndex].multiply(bone->getTransform()).multiply(_bindTransforms[boneIndex]);
    }
    
    /*
     Model space, original position --> [bindTransform]                 --> Bone space, bind position
     Bone space, bind position      --> [boneTransform]                 --> Model space, animated position
     */
    else if (bone->getTransformType() == VROBoneTransformType::Concatenated) {
        return bone->getTransform().multiply(_bindTransforms[boneIndex]);
    }
    
    /*
     With the local transform, each time we multiply by a bone's transform we end up in the bone local
     space of the parent bone. We have to therefore loop up and continually multiply by parent transforms
     until we reach the top-most bone. (This makes intuitive sense: the finger transform moves us to the
     arm, the arm transform to the torso, etc.).
     */
    else {
        std::shared_ptr<VROBone> bone;
        VROMatrix4f transform = _bindTransforms[boneIndex];
        VROMatrix4f inverseBind;
        
        while (boneIndex >= 0) {
            bone = _skeleton->getBone(boneIndex);
            transform = bone->getTransform().multiply(transform);
            inverseBind = _inverseBindTransforms[boneIndex];
            
            boneIndex = bone->getParentIndex();
        }
        return transform;
    }
}
