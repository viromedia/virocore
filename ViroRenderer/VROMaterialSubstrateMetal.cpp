//
//  VROMaterialSubstrateMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/30/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMaterialSubstrateMetal.h"
#if VRO_METAL

#include "VROSharedStructures.h"
#include "VROMetalUtils.h"
#include "VRODriverMetal.h"
#include "VROMatrix4f.h"
#include "VROLight.h"
#include "VROMath.h"
#include "VROAllocationTracker.h"
#include "VROConcurrentBuffer.h"
#include "VRORenderParameters.h"
#include "VROSortKey.h"
#include "VRORenderContext.h"

static std::map<std::string, std::shared_ptr<VROMetalShader>> _sharedPrograms;

std::shared_ptr<VROMetalShader> VROMaterialSubstrateMetal::getPooledShader(std::string vertexShader,
                                                                           std::string fragmentShader,
                                                                           id <MTLLibrary> library) {
    std::string name = vertexShader + "_" + fragmentShader;
    
    std::map<std::string, std::shared_ptr<VROMetalShader>>::iterator it = _sharedPrograms.find(name);
    if (it == _sharedPrograms.end()) {
        id <MTLFunction> vertexProgram = [library newFunctionWithName:[NSString stringWithUTF8String:vertexShader.c_str()]];
        id <MTLFunction> fragmentProgram = [library newFunctionWithName:[NSString stringWithUTF8String:fragmentShader.c_str()]];
        
        std::shared_ptr<VROMetalShader> program = std::make_shared<VROMetalShader>(vertexProgram, fragmentProgram);
        _sharedPrograms[name] = program;
        
        return program;
    }
    else {
        return it->second;
    }
}

VROMaterialSubstrateMetal::VROMaterialSubstrateMetal(const VROMaterial &material,
                                                     VRODriverMetal &driver) :
    _material(material),
    _lightingModel(material.getLightingModel()) {

    id <MTLDevice> device = driver.getDevice();
    id <MTLLibrary> library = driver.getLibrary();
    
    _lightingUniformsBuffer = new VROConcurrentBuffer(sizeof(VROSceneLightingUniforms), @"VROSceneLightingUniformBuffer", device);
    _materialUniformsBuffer = new VROConcurrentBuffer(sizeof(VROMaterialUniforms), @"VROMaterialUniformBuffer", device);
    
    switch (material.getLightingModel()) {
        case VROLightingModel::Constant:
            loadConstantLighting(material, library, device, driver);
            break;
            
        case VROLightingModel::Blinn:
            loadBlinnLighting(material, library, device, driver);
            break;
            
        case VROLightingModel::Lambert:
            loadLambertLighting(material, library, device, driver);
            break;
            
        case VROLightingModel::Phong:
            loadPhongLighting(material, library, device, driver);
            break;
            
        default:
            break;
    }
        
    ALLOCATION_TRACKER_ADD(MaterialSubstrates, 1);
}

VROMaterialSubstrateMetal::~VROMaterialSubstrateMetal() {
    delete (_materialUniformsBuffer);
    delete (_lightingUniformsBuffer);
    
    ALLOCATION_TRACKER_SUB(MaterialSubstrates, 1);
}

void VROMaterialSubstrateMetal::loadConstantLighting(const VROMaterial &material,
                                                     id <MTLLibrary> library, id <MTLDevice> device,
                                                     VRODriverMetal &driver) {
    
    
    std::string vertexProgram = "constant_lighting_vertex";
    std::string fragmentProgram;
    
    VROMaterialVisual &diffuse = material.getDiffuse();

    if (diffuse.getTextureType() == VROTextureType::None) {
        fragmentProgram = "constant_lighting_fragment_c";
    }
    else if (diffuse.getTextureType() == VROTextureType::Texture2D) {
        _textures.push_back(diffuse.getTexture());
        fragmentProgram = "constant_lighting_fragment_t";
    }
    else {
        _textures.push_back(diffuse.getTexture());
        fragmentProgram = "constant_lighting_fragment_q";
    }
    
    _program = getPooledShader(vertexProgram, fragmentProgram, library);
}

void VROMaterialSubstrateMetal::loadLambertLighting(const VROMaterial &material,
                                                    id <MTLLibrary> library, id <MTLDevice> device,
                                                    VRODriverMetal &driver) {
    
    std::string vertexProgram = "lambert_lighting_vertex";
    std::string fragmentProgram;
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    VROMaterialVisual &reflective = material.getReflective();
    
    if (diffuse.getTextureType() == VROTextureType::None) {
        if (reflective.getTextureType() == VROTextureType::TextureCube) {
            _textures.push_back(reflective.getTexture());
            fragmentProgram = "lambert_lighting_fragment_c_reflect";
        }
        else {
            fragmentProgram = "lambert_lighting_fragment_c";
        }
    }
    else {
        _textures.push_back(diffuse.getTexture());
        
        if (reflective.getTextureType() == VROTextureType::TextureCube) {
            _textures.push_back(reflective.getTexture());
            fragmentProgram = "lambert_lighting_fragment_t_reflect";
        }
        else {
            fragmentProgram = "lambert_lighting_fragment_t";
        }
    }
    
    _program = getPooledShader(vertexProgram, fragmentProgram, library);
}

void VROMaterialSubstrateMetal::loadPhongLighting(const VROMaterial &material,
                                                  id <MTLLibrary> library, id <MTLDevice> device,
                                                  VRODriverMetal &driver) {
    
    /*
     If there's no specular map, then we fall back to Lambert lighting.
     */
    VROMaterialVisual &specular = material.getSpecular();
    if (specular.getTextureType() != VROTextureType::Texture2D) {
        loadLambertLighting(material, library, device, driver);
        return;
    }
    
    std::string vertexProgram = "phong_lighting_vertex";
    std::string fragmentProgram;
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    VROMaterialVisual &reflective = material.getReflective();
    
    if (diffuse.getTextureType() == VROTextureType::None) {
        _textures.push_back(specular.getTexture());
        
        if (reflective.getTextureType() == VROTextureType::TextureCube) {
            _textures.push_back(reflective.getTexture());
            fragmentProgram = "phong_lighting_fragment_c_reflect";
        }
        else {
            fragmentProgram = "phong_lighting_fragment_c";
        }
    }
    else {
        _textures.push_back(diffuse.getTexture());
        _textures.push_back(specular.getTexture());
        
        if (reflective.getTextureType() == VROTextureType::TextureCube) {
            _textures.push_back(reflective.getTexture());
            fragmentProgram = "phong_lighting_fragment_t_reflect";
        }
        else {
            fragmentProgram = "phong_lighting_fragment_t";
        }
    }
    
    _program = getPooledShader(vertexProgram, fragmentProgram, library);
}

void VROMaterialSubstrateMetal::loadBlinnLighting(const VROMaterial &material,
                                                  id <MTLLibrary> library, id <MTLDevice> device,
                                                  VRODriverMetal &driver) {
    
    /*
     If there's no specular map, then we fall back to Lambert lighting.
     */
    VROMaterialVisual &specular = material.getSpecular();
    if (specular.getTextureType() != VROTextureType::Texture2D) {
        loadLambertLighting(material, library, device, driver);
        return;
    }
    
    std::string vertexProgram = "blinn_lighting_vertex";
    std::string fragmentProgram;
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    VROMaterialVisual &reflective = material.getReflective();
    
    if (diffuse.getTextureType() == VROTextureType::None) {
        _textures.push_back(specular.getTexture());

        if (reflective.getTextureType() == VROTextureType::TextureCube) {
            _textures.push_back(reflective.getTexture());
            fragmentProgram = "blinn_lighting_fragment_c_reflect";
        }
        else {
            fragmentProgram = "blinn_lighting_fragment_c";
        }
    }
    else {
        _textures.push_back(diffuse.getTexture());
        _textures.push_back(specular.getTexture());

        if (reflective.getTextureType() == VROTextureType::TextureCube) {
            _textures.push_back(reflective.getTexture());
            fragmentProgram = "blinn_lighting_fragment_t_reflect";
        }
        else {
            fragmentProgram = "blinn_lighting_fragment_t";
        }
    }
    
    _program = getPooledShader(vertexProgram, fragmentProgram, library);
}

VROConcurrentBuffer &VROMaterialSubstrateMetal::bindMaterialUniforms(float opacity, VROEyeType eye,
                                                                     int frame) {
    VROMaterialUniforms *uniforms = (VROMaterialUniforms *)_materialUniformsBuffer->getWritableContents(eye, frame);
    uniforms->diffuse_surface_color = toVectorFloat4(_material.getDiffuse().getColor());
    uniforms->diffuse_intensity = _material.getDiffuse().getIntensity();
    uniforms->shininess = _material.getShininess();
    uniforms->alpha = _material.getTransparency() * opacity;
    
    return *_materialUniformsBuffer;
}

void VROMaterialSubstrateMetal::updateSortKey(VROSortKey &key) const {
    key.shader = _program->getShaderId();
    key.textures = hashTextures(_textures);
}

void VROMaterialSubstrateMetal::bindShader() {
    // Do nothing in Metal, consider changing this to binding pipeline state?
    // The problem is that pipeline state in metal emcompasses both shader and
    // vertex layout
}

void VROMaterialSubstrateMetal::bindLights(int lightsHash,
                                           const std::vector<std::shared_ptr<VROLight>> &lights,
                                           const VRORenderContext &context,
                                           VRODriver &driver) {
    
    VRODriverMetal &metal = (VRODriverMetal &)driver;
    id <MTLRenderCommandEncoder> renderEncoder = metal.getRenderTarget()->getRenderEncoder();
    
    VROEyeType eyeType = context.getEyeType();
    int frame = context.getFrame();
    
    VROSceneLightingUniforms *uniforms = (VROSceneLightingUniforms *)_lightingUniformsBuffer->getWritableContents(eyeType,
                                                                                                                  frame);
    uniforms->num_lights = 0;
    VROVector3f ambientLight;
    
    for (const std::shared_ptr<VROLight> &light : lights) {
        if (light->getType() == VROLightType::Ambient) {
            ambientLight += light->getColor();
        }
        else {
            VROLightUniforms &light_uniforms = uniforms->lights[uniforms->num_lights];
            light_uniforms.type = (int) light->getType();
            light_uniforms.color = toVectorFloat3(light->getColor());
            light_uniforms.position = toVectorFloat3(light->getTransformedPosition());
            light_uniforms.direction = toVectorFloat3(light->getDirection());
            light_uniforms.attenuation_start_distance = light->getAttenuationStartDistance();
            light_uniforms.attenuation_end_distance = light->getAttenuationEndDistance();
            light_uniforms.attenuation_falloff_exp = light->getAttenuationFalloffExponent();
            light_uniforms.spot_inner_angle = degrees_to_radians(light->getSpotInnerAngle());
            light_uniforms.spot_outer_angle = degrees_to_radians(light->getSpotOuterAngle());
            
            uniforms->num_lights++;
        }
    }
    
    uniforms->ambient_light_color = toVectorFloat3(ambientLight);
    
    [renderEncoder setVertexBuffer:_lightingUniformsBuffer->getMTLBuffer(eyeType)
                            offset:_lightingUniformsBuffer->getWriteOffset(frame)
                           atIndex:3];
    [renderEncoder setFragmentBuffer:_lightingUniformsBuffer->getMTLBuffer(eyeType)
                              offset:_lightingUniformsBuffer->getWriteOffset(frame)
                             atIndex:0];
}

uint32_t VROMaterialSubstrateMetal::hashTextures(const std::vector<std::shared_ptr<VROTexture>> &textures) const {
    uint32_t h = 0;
    for (const std::shared_ptr<VROTexture> &texture : textures) {
        h = 31 * h + texture->getTextureId();
    }
    return h;
}

#endif
