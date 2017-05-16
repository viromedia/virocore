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

#include <memory>
#include <vector>

/*
 The VROSkeleton is the bone-structure that defines the control points
 for a skeletal animation. These control points (the bones) are the animatable
 objects that drive skeletal animations.
 
 To associate a skeleton with a geometry, use a VROSkinner.
 */
class VROSkeleton {
    
public:
    
    VROSkeleton(std::vector<std::shared_ptr<VROBone>> bones) :
        _bones(bones) {
        
    }
    virtual ~VROSkeleton() {}
    
    const std::shared_ptr<VROBone> getBone(int index) const {
        return _bones[index];
    }
    
private:
    
    /*
     The bones representing the skeleton. Each represents a control point of
     the animation. Moving a bone deforms the surface of this skinner's
     geometry.
     */
    std::vector<std::shared_ptr<VROBone>> _bones;
    
};

#endif /* VROSkeleton_h */
