//
//  VROLightingUBO.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/11/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROLightingUBO_h
#define VROLightingUBO_h

#include "VROOpenGL.h"
#include <vector>
#include <atomic>
#include <memory>

// Grouped in 4N slots, matching lighting_general_functions.glsl
typedef struct {
    int type;
    float attenuation_start_distance;
    float attenuation_end_distance;
    float attenuation_falloff_exp;
    
    float position[4];
    float direction[4];
    
    float color[3];
    float spot_inner_angle;
    
    float spot_outer_angle;
    float padding3;
    float padding4;
    float padding5;
} VROLightData;

typedef struct {
    int num_lights;
    float padding0, padding1, padding2;
    
    float ambient_light_color[4];
    VROLightData lights[8];
} VROLightingData;

class VROLight;
class VROShaderProgram;
class VRODriverOpenGL;

/*
 Manages the UBOs (Uniform Buffer Objects) that link lights to shaders.
 UBOs enable us to share data among shaders, so we only have to bind 
 the new light settings when the lights themselves change, or when we 
 change what lights we're rendering. We do not have to rebind every time
 the shader changes.
 
 UBOs have three components: the UBO itself (_lightingUBO), a binding
 point (_lightingUBOBindingPoint), and a block index (_lightingBlockIndex).
 
 The _lightingUBO is similar to a VBO: it's essentially a block of GPU
 memory. Unlike a VBO, however, it can be read in GLSL shaders as 
 structured data. The VROLightingData struct, above, matches the lighting
 layout in lighting_general_functions.glsl. We write into _lightingUBO
 whenever the lights change, updating the fields.
 
 The _lightingUBOBindingPoint is similar to a texture unit. It is independent
 of shaders, and there are fixed number of these in our EGL context.
 Every time we create a new UBO, we have to create a new binding point, 
 and bind the UBO to said binding point via glBindBufferBase.
 
 Finally, the _lightingBlockIndex is *shader-specific*; it's found in 
 VROShaderProgram. It's similar to a shader's uniform location. When we
 compile a shader with a layout in it, that layout is given an index in 
 the shader, much like any uniform. We retrieve this location via 
 glGetUniformBlockIndex. Once we have this location, we can bind it to a
 specific UBO by invoking glUniformBlockBinding. This binds a shader's 
 uniform layout to a specific UBO binding point. The binding point, in 
 turn, was earlier bound to a specific UBO.
 
 What does all of this enable?
 
 1. We can have multiple batches of lights, each represented by their 
    own UBO. To switch from one batch to another, we just have to call 
    glUniformBlockBinding, and the shader will now point to a new batch.
 
 2. When we update lights, we don't have to update every shader. We just 
    glBufferSubData into the _lightingUBO for each batch that uses those
    lights.
 
 Finally, quick note on why binding points exist. Why not directly bind 
 uniform blocks in shaders to UBOs? Why do we need this extra level of 
 indirection? The answer is the same reason texture units exist: hardware
 limitations. It's the same reason we don't directly map shader texture 
 uniforms to texture IDs; we instead map them to texture units. This is 
 because there can only be so many units of memory on the GPU used 
 simultaneously for these things. Binding points enforce the limit.
 */
class VROLightingUBO {
    
public:
    
    static void unbind(std::shared_ptr<VROShaderProgram> &program);
    
    VROLightingUBO(int hash, const std::vector<std::shared_ptr<VROLight>> &lights,
                   std::shared_ptr<VRODriverOpenGL> driver);
    virtual ~VROLightingUBO();
    
    /*
     Bind this lighting UBO into the "lighting" block for the given program.
     */
    void bind(std::shared_ptr<VROShaderProgram> &program);
    
    /*
     Invoke to indicate that a light in this UBO has changed. When this occurs
     we have to rewrite the lights to the UBO.
     */
    void setNeedsUpdate() {
        _needsUpdate = true;
    }
    
    /*
     Hash which uniquely identifies the set of lights composing this UBO.
     */
    int getHash() const {
        return _hash;
    }
    
private:
    
    int _hash;
    
    /*
     The uniform buffer object ID and binding point for lighting parameters.
     */
    GLuint _lightingUBO;
    int _lightingUBOBindingPoint = 0;
    
    /*
     The lights that are a part of this UBO.
     */
    std::vector<std::shared_ptr<VROLight>> _lights;
    
    /*
     The driver that created this UBO.
     */
    std::weak_ptr<VRODriverOpenGL> _driver;
    
    /*
     True if a light in this UBO has changed.
     */
    bool _needsUpdate;
    
    /*
     Update the lights in this UBO, rewriting them to the buffer.
     */
    void updateLights();
    
};

#endif
