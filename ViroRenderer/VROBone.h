//
//  VROBone.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROBone_h
#define VROBone_h

#include "VROMatrix4f.h"
#include "VROAnimatable.h"

/*
 VROBones are the control points for skeletal animation. They are animatable: 
 to drive a skeletal animation, invoke setTransform() for the bones that should
 be animated within an animation block. This will animate the bones, which in
 turn animates the VROSkeleton, which in turn (by way of VROSkinner objects) 
 animates any attached VROGeometry objects.
 
 The transform of each bone is a *concatenated* transform that moves a bone
 (and in turn any associated mesh vertices) from its bind position in bone local
 space, to its animated position, also in bone local space.
 
 Typically the transform is a concatenation of all transforms moving down the
 skeleton hierarchy: e.g., the transform for the finger bone is a concatenation
 of the upper arm transform, the lower arm transform, the hand transform, and 
 finally the finger transform.
 
 The input coordinates to the bone tranform are assumed to be in bone local
 space, bind position. The bindTransform in each VROSkinner is responsible for
 transforming geometries into this space and position. See VROSkinner.h for more
 details.
 */
class VROBone : public VROAnimatable {
    
public:
    
    VROBone(int parentIndex) :
        _parentIndex(parentIndex) {
    }
    virtual ~VROBone() {}
    
    void setTransform(VROMatrix4f transform);
    
    int getParentIndex() const {
        return _parentIndex;
    }
    VROMatrix4f getTransform() const {
        return _transform;
    }

private:
    
    /*
     The index of this node's parent in the skeleton. This is an index into the parent
     VROSkeleton's _bones array.
     */
    int _parentIndex;

    /*
     The transform from this bone's bind position in its bone local space, to its
     animated position in bone local space.
     */
    VROMatrix4f _transform;
    
};

#endif /* VROBone_h */
