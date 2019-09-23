//
//  VROSkinner.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
     The geometryBindTransform moves the model into alignment with the skeleton.
     The boneSpaceTransforms moves each bone into the "T-pose" bind position in
     bone space. Use these to construct the _bindTransforms and _inverseBindTransforms
     for each bone. These matrices go to and from the encoded position in model space
     to the bind position in bone local space.
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
