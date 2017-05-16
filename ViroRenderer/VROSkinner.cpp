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

VROMatrix4f VROSkinner::getModelTransform(int boneIndex) {
    /*
     To transform from a vertex from its encoded position in model space to its
     animated position in model space, we must:
     
     1. Transform the vertex into the bind position of the skeleton.
     2. Transform the vertex into the joint local coordinate space of the bone.
        (steps 1 and 2 are both acheived via the _bindTransform)
     3. Transform the vertex by bone transform[i], which moves it into
        the animated position of bone[i] and the coordinate space of bone[i]'s
        parent.
     4. Repeat step 3 for bone[i]'s parent, until we reach the root of the skeleton.
        Upon reaching the root, we are back in model space, and in the animated
        position.
     */
    const std::shared_ptr<VROBone> &bone = _skeleton->getBone(boneIndex);
    int parentIndex = bone->getParentIndex();
    
    VROMatrix4f result = bone->getTransform().multiply(_bindTransforms[boneIndex]);
    while (parentIndex >= 0) {
        const std::shared_ptr<VROBone> &parent = _skeleton->getBone(parentIndex);
        
        result = parent->getTransform().multiply(result);
        parentIndex = parent->getParentIndex();
    }
    
    return result;
}
