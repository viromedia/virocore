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
#include "VRONode.h"

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

VROMatrix4f VROSkinner::getCurrentBoneWorldTransform(int boneId) {
    // TODO VIRO-4712: Account for Non-Legacy Bones when setting bone transforms
    if (_skeleton->getBone(boneId)->getTransformType() != VROBoneTransformType::Legacy) {
        pwarn("Unable to grab world transform of non-Legacy bone types");
        return VROMatrix4f::identity();
    }

    // Grab the skinner's node transform
    std::shared_ptr<VRONode> skinnerNode = _skinnerNodeWeak.lock();
    VROMatrix4f skinnerWorldTrans = VROMatrix4f::identity();
    if (skinnerNode != nullptr) {
        skinnerWorldTrans = skinnerNode->getWorldTransform();
    }

    // Grab the bone's transform, convert it into geometry and then finally world space.
    const std::shared_ptr<VROBone> &bone = _skeleton->getBone(boneId);
    VROMatrix4f transform = _inverseBindTransforms[boneId].multiply(bone->getTransform());
    return skinnerWorldTrans.multiply(transform);
}

void VROSkinner::setCurrentBoneWorldTransform(int boneId, VROMatrix4f targetWorldTrans, bool recurse) {
    // TODO VIRO-4712: Account for Non-Legacy Bones when setting bone transforms
    if (_skeleton->getBone(boneId)->getTransformType() != VROBoneTransformType::Legacy) {
        pwarn("Unable to set world transform of non-Legacy bone types");
        return;
    }

    // First grab the original bone transform in case we'll need to use it later
    VROMatrix4f originalTransform = getCurrentBoneWorldTransform(boneId);

    // Grab the skinner's node transform
    VROMatrix4f skinnerWorldTrans = VROMatrix4f::identity();
    std::shared_ptr<VRONode> skinnerNode = _skinnerNodeWeak.lock();
    if (skinnerNode != nullptr) {
        skinnerWorldTrans = skinnerNode->getWorldTransform();
    }

    // Convert the desired targetWorldTransform into skinner node space
    VROMatrix4f targetSkinnerTrans = skinnerWorldTrans.invert().multiply(targetWorldTrans);

    // Convert targetSkinnerTransform into boneSpace - say targetedBoneSpace (note this is
    // done by inverseBind.invert). Since the bindTransform already considers the starting
    // geometryBounded bone transform, this also produces the bone delta tha twe can set
    // on the skeleton.
    VROMatrix4f delta = _inverseBindTransforms[boneId].invert().multiply(targetSkinnerTrans);
    const std::shared_ptr<VROBone> &joint = _skeleton->getBone(boneId);
    joint->setTransform(delta, VROBoneTransformType::Legacy);

    // If we are not recursing this calculation down to child bones, return.
    if (!recurse) {
        return;
    }

    // Grab all the child bones from the current boneId.
    std::vector<int> childBoneIndexes;
    for (int i = 0; i < _skeleton->getNumBones(); i ++) {
        if (_skeleton->getBone(i)->getParentIndex() == boneId && i != boneId) {
            childBoneIndexes.push_back(i);
        }
    }

    if (childBoneIndexes.size() <=0) {
        return;
    }

    // Now calculate the child's transform in reference to the parent.
    for (auto childBoneIndex : childBoneIndexes) {
        VROMatrix4f childWorldTrans = getCurrentBoneWorldTransform(childBoneIndex);
        VROMatrix4f transformToChild = originalTransform.invert().multiply(childWorldTrans);
        VROMatrix4f newChildTransform = targetWorldTrans.multiply(transformToChild);
        setCurrentBoneWorldTransform(childBoneIndex, newChildTransform, recurse);
    }
}