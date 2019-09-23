//
//  VROBoneConstraint.cpp
//  ViroRenderer
//
//  Copyright © 2019 Viro Media. All rights reserved.
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
