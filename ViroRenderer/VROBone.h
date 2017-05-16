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
 to drive a skeletal animation, invoke setTranform() for the bones that should 
 be animated within an animation block. This will animate the bones, which in
 turn animates the VROSkeleton, which in turn (by way of VROSkinner objects) 
 animates any attached VROGeometry objects.
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
     The transform from this bone's bind position in its joint local space, to its
     animated position in the joint local space of its parent bone.
     
     In other words, this transform moves from the bind position to the animated
     position, and moves *up* one coordinate space in the skeleton. 
     
     Therefore, by concatenating all bone transforms together hierarchically, we can
     unwind local bone transforms into a full skeletal transform, back in the coordinate 
     space of the skeleton (and geometry).
     
     Animatable: changing this value drives the skeletal animation.
     */
    VROMatrix4f _transform;
    
};

#endif /* VROBone_h */
