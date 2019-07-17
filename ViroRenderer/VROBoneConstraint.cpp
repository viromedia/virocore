//
//  VROBoneConstraint.cpp
//  ViroRenderer
//
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROBoneConstraint.h"
#include "VROVector3f.h"
#include "VRONode.h"
#include "VROSkeleton.h"
#include "VROBone.h"

VROBoneConstraint::VROBoneConstraint(std::shared_ptr<VROSkeleton> skeleton,
                                     int boneIndex,
                                     VROMatrix4f constrainedOffsetTransform) {
    _skeleton = skeleton;
    _constraintedBoneIndex = boneIndex;
    _constrainedOffsetTransform = constrainedOffsetTransform;
}

VROMatrix4f VROBoneConstraint::getTransform(const VRORenderContext &context, VROMatrix4f nodeWorldTransform) {
    std::shared_ptr<VROSkeleton> skeleton = _skeleton.lock();
    if (skeleton == nullptr) {
        return nodeWorldTransform;
    }

    // Grab the bone transforms in model space.
    VROMatrix4f modelWorldTrans = skeleton->getSkinnerRootNode()->getWorldTransform();
    VROMatrix4f boneWorldTrans = skeleton->getCurrentBoneWorldTransform(_constraintedBoneIndex);
    VROMatrix4f boneTransInModelSpace = boneWorldTrans * modelWorldTrans.invert();

    // Apply any transform offsets this attachment constraint has.
    VROMatrix4f boneTransInModelSpaceShifted = boneTransInModelSpace * _constrainedOffsetTransform;

    // Return the shifted bone in world space.
    return boneTransInModelSpaceShifted * modelWorldTrans;
}
