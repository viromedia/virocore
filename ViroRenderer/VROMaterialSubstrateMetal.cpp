//
//  VROMaterialSubstrateMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/30/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMaterialSubstrateMetal.h"
#include "VROSharedStructures.h"
#include "VROMetalUtils.h"
#include "VRORenderContextMetal.h"
#include "VROMatrix4f.h"
#include "VROLight.h"
#include "VROMath.h"

VROMaterialSubstrateMetal::VROMaterialSubstrateMetal(VROMaterial &material,
                                                     const VRORenderContextMetal &context) :
    _material(material),
    _lightingModel(material.getLightingModel()) {

    id <MTLDevice> device = context.getDevice();
    id <MTLLibrary> library = context.getLibrary();
    
    _lightingUniformsBuffer = [device newBufferWithLength:sizeof(VROSceneLightingUniforms) options:0];
    _lightingUniformsBuffer.label = @"VROSceneLightingUniformBuffer";
    
    _materialUniformsBuffer = [device newBufferWithLength:sizeof(VROMaterialUniforms) options:0];
    _materialUniformsBuffer.label = @"VROMaterialUniformBuffer";
    
    switch (material.getLightingModel()) {
        case VROLightingModel::Constant:
            loadConstantLighting(material, library, device, context);
            break;
            
        case VROLightingModel::Blinn:
            loadBlinnLighting(material, library, device, context);
            break;
            
        case VROLightingModel::Lambert:
            loadLambertLighting(material, library, device, context);
            break;
            
        case VROLightingModel::Phong:
            loadPhongLighting(material, library, device, context);
            break;
            
        default:
            break;
    }
}

VROMaterialSubstrateMetal::~VROMaterialSubstrateMetal() {
    
}

void VROMaterialSubstrateMetal::loadConstantLighting(VROMaterial &material,
                                                     id <MTLLibrary> library, id <MTLDevice> device,
                                                     const VRORenderContextMetal &context) {
    

    _vertexProgram   = [library newFunctionWithName:@"constant_lighting_vertex"];
    VROMaterialVisual &diffuse = material.getDiffuse();

    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        _fragmentProgram = [library newFunctionWithName:@"constant_lighting_fragment_c"];
    }
    else {
        _textures.push_back(diffuse.getContentsTexture());
        _fragmentProgram = [library newFunctionWithName:@"constant_lighting_fragment_t"];
    }
}

void VROMaterialSubstrateMetal::loadLambertLighting(VROMaterial &material,
                                                    id <MTLLibrary> library, id <MTLDevice> device,
                                                    const VRORenderContextMetal &context) {
    
    _vertexProgram   = [library newFunctionWithName:@"lambert_lighting_vertex"];
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        _fragmentProgram = [library newFunctionWithName:@"lambert_lighting_fragment_c"];
    }
    else {
        _textures.push_back(diffuse.getContentsTexture());
        _fragmentProgram = [library newFunctionWithName:@"lambert_lighting_fragment_t"];
    }
}

void VROMaterialSubstrateMetal::loadPhongLighting(VROMaterial &material,
                                                  id <MTLLibrary> library, id <MTLDevice> device,
                                                  const VRORenderContextMetal &context) {
    
    /*
     If there's no specular map, then we fall back to Lambert lighting.
     */
    VROMaterialVisual &specular = material.getSpecular();
    if (specular.getContentsType() != VROContentsType::Texture2D) {
        loadLambertLighting(material, library, device, context);
        return;
    }
    
    _vertexProgram   = [library newFunctionWithName:@"phong_lighting_vertex"];
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        _fragmentProgram = [library newFunctionWithName:@"phong_lighting_fragment_c"];
    }
    else {
        _textures.push_back(diffuse.getContentsTexture());
        _fragmentProgram = [library newFunctionWithName:@"phong_lighting_fragment_t"];
    }
    _textures.push_back(specular.getContentsTexture());
}

void VROMaterialSubstrateMetal::loadBlinnLighting(VROMaterial &material,
                                                  id <MTLLibrary> library, id <MTLDevice> device,
                                                  const VRORenderContextMetal &context) {
    
    /*
     If there's no specular map, then we fall back to Lambert lighting.
     */
    VROMaterialVisual &specular = material.getSpecular();
    if (specular.getContentsType() != VROContentsType::Texture2D) {
        loadLambertLighting(material, library, device, context);
        return;
    }
    
    _vertexProgram   = [library newFunctionWithName:@"blinn_lighting_vertex"];
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        _fragmentProgram = [library newFunctionWithName:@"blinn_lighting_fragment_c"];
    }
    else {
        _textures.push_back(diffuse.getContentsTexture());
        _fragmentProgram = [library newFunctionWithName:@"blinn_lighting_fragment_t"];
    }
    _textures.push_back(specular.getContentsTexture());
}

void VROMaterialSubstrateMetal::setMaterialUniforms() {
    VROMaterialUniforms *uniforms = (VROMaterialUniforms *)[_materialUniformsBuffer contents];
    uniforms->diffuse_surface_color = toVectorFloat4(_material.getDiffuse().getContentsColor());
    uniforms->diffuse_intensity = _material.getDiffuse().getIntensity();
    uniforms->shininess = _material.getShininess();
    uniforms->alpha = _material.getTransparency();
}

void VROMaterialSubstrateMetal::setLightingUniforms(const std::vector<std::shared_ptr<VROLight>> &lights) {
    VROSceneLightingUniforms *uniforms = (VROSceneLightingUniforms *)[_lightingUniformsBuffer contents];
    uniforms->num_lights = (int) lights.size();
    
    VROVector3f ambientLight;

    for (int i = 0; i < lights.size(); i++) {
        const std::shared_ptr<VROLight> &light = lights[i];
        
        VROLightUniforms &light_uniforms = uniforms->lights[i];
        light_uniforms.type = (int) light->getType();
        light_uniforms.position = toVectorFloat3(light->getTransformedPosition());
        light_uniforms.direction = toVectorFloat3(light->getDirection());
        light_uniforms.color = toVectorFloat3(light->getColor());
        light_uniforms.attenuation_start_distance = light->getAttenuationStartDistance();
        light_uniforms.attenuation_end_distance = light->getAttenuationEndDistance();
        light_uniforms.attenuation_falloff_exp = light->getAttenuationFalloffExponent();
        light_uniforms.spot_inner_angle = degrees_to_radians(light->getSpotInnerAngle());
        light_uniforms.spot_outer_angle = degrees_to_radians(light->getSpotOuterAngle());
        
        if (light->getType() == VROLightType::Ambient) {
            ambientLight += light->getColor();
        }
    }
    
    uniforms->ambient_light_color = toVectorFloat3(ambientLight);
}