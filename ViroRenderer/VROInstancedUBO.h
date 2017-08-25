//
//  VROInstancedUBO.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROInstancedUBO_h
#define VROInstancedUBO_h

#include <memory>
#include <vector>
#include "VROMatrix4f.h"
#include "VROVector4f.h"
#include "VROOpenGL.h"
#include "VROVector3f.h"
#include "VROBoundingBox.h"

class VROShaderProgram;
class VRODriver;
class VROShaderModifier;

/*
 VROInstancedUBO facilitates the binding and updating of uniform buffer objects (for batching
 data to the GPU), and as well as shader modifiers (for processing batched data) that are
 both required for instance rendering.
 */
class VROInstancedUBO {
public:
    VROInstancedUBO(){};
    virtual ~VROInstancedUBO(){};

    /*
     Functions for Binding / unbinding UBOs to the currently active shader program.
     */
    virtual void bind(std::shared_ptr<VROShaderProgram> &program) = 0;
    virtual void unbind(std::shared_ptr<VROShaderProgram> &program) = 0;

    /*
     Creates fragment and/or vertex shader modifiers to process per-instance data bounded
     by this UBO when rendering. Instances can be referred to with the "v_instance_id",
     for example: geometry_model_matrix = particles_mat4_transform[v_instance_id];
     */
    virtual std::vector<std::shared_ptr<VROShaderModifier>> createInstanceShaderModifier() = 0;

    /*
     Returns the total number of draw calls needed to bind/render all per-instance data batched in
     this VROInstanced UBO. This is usually used for drawing large number of instances beyond the
     capacity of a single UBO.
     */
    virtual int getNumberOfDrawCalls() = 0;

    /*
     Update the uniform buffers with a set of per-instance data corresponding to the given
     current draw call index. We then return the number of instances that were updated, so
     that the renderer may draw them.
     */
    virtual int bindDrawData(int currentDrawCallIndex) = 0;

    /*
     Bounding box representing the region of space encapsulating all instanced drawn objects.
     */
    virtual VROBoundingBox getInstancedBoundingBox() = 0;
};

#endif /* VROInstancedUBO_h */
