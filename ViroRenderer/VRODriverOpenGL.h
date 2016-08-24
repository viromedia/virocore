//
//  VRODriverOpenGL.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODriverOpenGL_h
#define VRODriverOpenGL_h

#include "VRODriver.h"
#include "VROGeometrySubstrateOpenGL.h"
#include "VROMaterialSubstrateOpenGL.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROVideoTextureCacheOpenGL.h"
#include "VROShaderProgram.h"

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

class VRODriverOpenGL : public VRODriver {
    
public:
    
    VRODriverOpenGL(EAGLContext *eaglContext) :
        _eaglContext(eaglContext) {
        
        glGenBuffers(1, &_lightingUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, _lightingUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(VROLightingData), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
            
        glBindBufferBase(GL_UNIFORM_BUFFER, _lightingUBOBindingPoint, _lightingUBO);
    }
    
    VROGeometrySubstrate *newGeometrySubstrate(const VROGeometry &geometry) {
        return new VROGeometrySubstrateOpenGL(geometry, *this);
    }
    
    VROMaterialSubstrate *newMaterialSubstrate(VROMaterial &material) {
        return new VROMaterialSubstrateOpenGL(material, *this);
    }
    
    VROTextureSubstrate *newTextureSubstrate(VROTextureType type, std::vector<UIImage *> &images) {
        return new VROTextureSubstrateOpenGL(type, images, *this);
    }
    
    VROTextureSubstrate *newTextureSubstrate(VROTextureType type, VROTextureFormat format, std::shared_ptr<VROData> data,
                                             int width, int height) {
        return new VROTextureSubstrateOpenGL(type, format, data, width, height, *this);
    }
    
    VROTextureSubstrate *newTextureSubstrate(int width, int height, CGContextRef bitmapContext) {
        return new VROTextureSubstrateOpenGL(width, height, bitmapContext, *this);
    }
    
    VROVideoTextureCache *newVideoTextureCache() {
        return new VROVideoTextureCacheOpenGL(_eaglContext);
    }
    
    GLuint getLightingUBO() const {
        return _lightingUBO;
    }
    
    int getLightingUBOBindingPoint() const {
        return _lightingUBOBindingPoint;
    }
    
    std::shared_ptr<VROShaderProgram> getPooledShader(std::string vertexShader,
                                                      std::string fragmentShader,
                                                      const std::vector<std::string> &samplers) {
        std::string name = vertexShader + "_" + fragmentShader;
        
        std::map<std::string, std::shared_ptr<VROShaderProgram>>::iterator it = _sharedPrograms.find(name);
        if (it == _sharedPrograms.end()) {
            std::shared_ptr<VROShaderProgram> program = std::make_shared<VROShaderProgram>(vertexShader, fragmentShader,
                                                                                           ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            for (const std::string &sampler : samplers) {
                program->addSampler(sampler);
            }
            _sharedPrograms[name] = program;
            return program;
        }
        else {
            return it->second;
        }
    }
    
private:
    
    EAGLContext *_eaglContext;
    
    /*
     The uniform buffer object ID and binding point for lighting parameters.
     */
    GLuint _lightingUBO = 0;
    const int _lightingUBOBindingPoint = 0;
    
    /*
     Shader programs are shared across the system.
     */
    std::map<std::string, std::shared_ptr<VROShaderProgram>> _sharedPrograms;
    
};

#endif /* VRODriverOpenGL_h */
