//
//  VROSkinner.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROSkinner_h
#define VROSkinner_h

#include "VROMatrix4f.h"
#include "VROGeometrySource.h"
#include <vector>
#include <memory>

class VROGeometry;
class VROSkeleton;

/*
 VROSkinner is the base class for skeletal animation; it associates an animation
 skeleton with the geometry that will be deformed.
 
 A single VROSkeleton can be used by multiple VROGeometries; each geometry using
 the VROSkeleton will have its own VROSkinner that maps the geometry to the 
 skeleton.
 */
class VROSkinner {
    
public:
    
    VROSkinner(std::shared_ptr<VROSkeleton> skeleton,
               std::vector<VROMatrix4f> bindTransforms,
               std::shared_ptr<VROGeometrySource> boneIndices,
               std::shared_ptr<VROGeometrySource> boneWeights) :
        _skeleton(skeleton),
        _bindTransforms(bindTransforms),
        _boneIndices(boneIndices),
        _boneWeights(boneWeights) {}
    virtual ~VROSkinner() {}
    
    /*
     Get the concatenated transform that will transform a vertex tied to the
     given bone from its original (encoded) position in model space, to its animated
     position in model space.
     */
    VROMatrix4f getModelTransform(int boneIndex);
    
    std::shared_ptr<VROSkeleton> getSkeleton() {
        return _skeleton;
    }
    
    const std::shared_ptr<VROGeometrySource> getBoneIndices() const {
        return _boneIndices;
    }
    const std::shared_ptr<VROGeometrySource> getBoneWeights() const {
        return _boneWeights;
    }
    
private:
    
    /*
     The geometry being animated, with coordinates in model space.
     */
    std::shared_ptr<VROGeometry> _geometry;
    
    /*
     The skeleton that drives the animation. The skeleton is *also* in model space.
     */
    std::shared_ptr<VROSkeleton> _skeleton;
    
    /*
     The transforms from model space (the coordinate space of _geometry) to the
     "joint local space" of the given bone, in the bind position. In other words,
     this tranforms a vertex in _geometry to its position relative to the given
     bone.
     
     These transforms serve two purposes:
     
     1. They place the geometry into the bind pose, the pose at which the geometry
     aligns with the skeleton, and from which its vertices can therefore be animated
     alongside the skeleton's bones.
     
     2. They transform from model space into the coordinate system of a given bone.
     This way we can animate vertices hierarchically: e.g. a finger vertex can animate
     around the finger bone, then the elbow, then the shoulder, etc. See VROBone.h
     and getWorldTransform() for more details.
     
     Finally, note that the transform at _bindTransforms[i] is for the bone in
     _skeleton.bones[i].
     */
    std::vector<VROMatrix4f> _bindTransforms;
    
    /*
     Vertex data that maps each vertex to the bones that influence its position
     during skeletal animation. The indices map into the _skeleton's _bones array.
     */
    std::shared_ptr<VROGeometrySource> _boneIndices;
    
    /*
     Vertex data that for each vertex defines the weight each bone (mapped to
     via _boneIndices) has in influencing the vertex.
     */
    std::shared_ptr<VROGeometrySource> _boneWeights;
    
};

#endif /* VROSkinner_h */
