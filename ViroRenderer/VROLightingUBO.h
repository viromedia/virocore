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
