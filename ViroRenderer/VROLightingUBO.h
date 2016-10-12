//
//  VROLightingUBO.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/11/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES3/glext.h>
#import <vector>

// Incrementing binding points
static std::atomic_int sUBOBindingPoint;

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

class VROLightingUBO {
    
public:
    
    VROLightingUBO() :
        _lightingUBOBindingPoint(sUBOBindingPoint++) {
        
        glGenBuffers(1, &_lightingUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, _lightingUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(VROLightingData), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        
        // Links the UBO and the binding point
        glBindBufferBase(GL_UNIFORM_BUFFER, _lightingUBOBindingPoint, _lightingUBO);
    }
    
    /*
     Bind this lighting UBO into the "lighting" block for the given program.
     */
    void bind(std::shared_ptr<VROShaderProgram> &program);
    
    /*
     Write the given lights into this UBO.
     */
    void writeLights(const std::vector<std::shared_ptr<VROLight>> &lights);
    
private:
    
    /*
     The uniform buffer object ID and binding point for lighting parameters.
     */
    GLuint _lightingUBO;
    const int _lightingUBOBindingPoint = 0;
    
};
