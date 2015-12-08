//
//  VROMaterialSubstrateMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/30/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMaterialSubstrateMetal.h"
#include "SharedStructures.h"
#include "VROMetalUtils.h"
#include "VRORenderContextMetal.h"
#include "VROMatrix4f.h"
#include "VROLight.h"

VROMaterialSubstrateMetal::VROMaterialSubstrateMetal(VROMaterial &material,
                                                     const VRORenderContextMetal &context) {
    
    _lightingModel = material.getLightingModel();

    id <MTLDevice> device = context.getDevice();
    id <MTLLibrary> library = context.getLibrary();
    
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
    
    _lightingUniformsBuffer = [device newBufferWithLength:sizeof(VROConstantLightingUniforms) options:0];
    _lightingUniformsBuffer.label = @"VROConstantLightingUniformBuffer";
    
    VROConstantLightingUniforms *uniforms = (VROConstantLightingUniforms *)[_lightingUniformsBuffer contents];

    _vertexProgram   = [library newFunctionWithName:@"constant_lighting_vertex"];

    VROMaterialVisual &ambient = material.getAmbient();
    VROMaterialVisual &diffuse = material.getDiffuse();

    if (ambient.getContentsType() == VROContentsType::Fixed) {
        uniforms->ambient_color = toVectorFloat4(ambient.getContentsColor());

        if (diffuse.getContentsType() == VROContentsType::Fixed) {
            uniforms->diffuse_color = toVectorFloat4(diffuse.getContentsColor());
            _fragmentProgram = [library newFunctionWithName:@"constant_lighting_fragment_cc"];
        }
        else {
            _textures.push_back(((VROTextureSubstrateMetal *)diffuse.getContentsTexture()->getSubstrate(context))->getTexture());
            _fragmentProgram = [library newFunctionWithName:@"constant_lighting_fragment_ct"];
        }
    }
    else {
        _textures.push_back(((VROTextureSubstrateMetal *)ambient.getContentsTexture()->getSubstrate(context))->getTexture());

        if (diffuse.getContentsType() == VROContentsType::Fixed) {
            uniforms->diffuse_color = toVectorFloat4(diffuse.getContentsColor());
            _fragmentProgram = [library newFunctionWithName:@"constant_lighting_fragment_ct"];
        }
        else {
            _textures.push_back(((VROTextureSubstrateMetal *)diffuse.getContentsTexture()->getSubstrate(context))->getTexture());
            _fragmentProgram = [library newFunctionWithName:@"constant_lighting_fragment_tt"];
        }
    }
}

void VROMaterialSubstrateMetal::loadBlinnLighting(VROMaterial &material,
                                                  id <MTLLibrary> library, id <MTLDevice> device,
                                                  const VRORenderContextMetal &context) {
    
    _vertexProgram   = [library newFunctionWithName:@"blinn_lighting_vertex"];
    _fragmentProgram = [library newFunctionWithName:@"blinn_lighting_fragment"];
    
    _lightingUniformsBuffer = [device newBufferWithLength:sizeof(VROBlinnLightingUniforms) options:0];
    _lightingUniformsBuffer.label = @"VROBlinnLightingUniformBuffer";
    
    VROBlinnLightingUniforms *uniforms = (VROBlinnLightingUniforms *)[_lightingUniformsBuffer contents];

    VROMaterialVisual &ambient = material.getAmbient();
    uniforms->ambient_color = toVectorFloat4(ambient.getContentsColor());
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    uniforms->diffuse_color = toVectorFloat4(diffuse.getContentsColor());
    
}

void VROMaterialSubstrateMetal::loadPhongLighting(VROMaterial &material,
                                                  id <MTLLibrary> library, id <MTLDevice> device,
                                                  const VRORenderContextMetal &context) {
    
    _vertexProgram   = [library newFunctionWithName:@"phong_lighting_vertex"];
    _fragmentProgram = [library newFunctionWithName:@"phong_lighting_fragment"];
    
    _lightingUniformsBuffer = [device newBufferWithLength:sizeof(VROPhongLightingUniforms) options:0];
    _lightingUniformsBuffer.label = @"VROPhongLightingUniformBuffer";
    
    VROPhongLightingUniforms *uniforms = (VROPhongLightingUniforms *)[_lightingUniformsBuffer contents];
    //TODO
}

void VROMaterialSubstrateMetal::loadLambertLighting(VROMaterial &material,
                                                    id <MTLLibrary> library, id <MTLDevice> device,
                                                    const VRORenderContextMetal &context) {
    
    _vertexProgram   = [library newFunctionWithName:@"lambert_lighting_vertex"];
    _fragmentProgram = [library newFunctionWithName:@"lambert_lighting_fragment"];
    
    _lightingUniformsBuffer = [device newBufferWithLength:sizeof(VROLambertLightingUniforms) options:0];
    _lightingUniformsBuffer.label = @"VROLambertLightingUniformBuffer";
    
    VROLambertLightingUniforms *uniforms = (VROLambertLightingUniforms *)[_lightingUniformsBuffer contents];
    //TODO
}

void VROMaterialSubstrateMetal::setLightingUniforms(const std::shared_ptr<VROLight> &light) {
    switch (_lightingModel) {
        case VROLightingModel::Constant:
            bindConstantLighting(light);
            break;
            
        case VROLightingModel::Blinn:
            bindBlinnLighting(light);
            break;
            
        case VROLightingModel::Lambert:
            bindLambertLighting(light);
            break;
            
        case VROLightingModel::Phong:
            bindPhongLighting(light);
            break;
            
        default:
            break;
    }
}

void VROMaterialSubstrateMetal::bindConstantLighting(const std::shared_ptr<VROLight> &light) {
    VROConstantLightingUniforms *uniforms = (VROConstantLightingUniforms *)[_lightingUniformsBuffer contents];
    
    switch (light->getType()) {
        case VROLightType::Ambient:
            uniforms->ambient_light = toVectorFloat4(light->getColor());
            break;
            
        case VROLightType::Directional:
            
            break;
            
        case VROLightType::Omni:
            
            break;
            
        case VROLightType::Spot:
            
            break;
            
        default:
            break;
    }
}

void VROMaterialSubstrateMetal::bindBlinnLighting(const std::shared_ptr<VROLight> &light) {
    VROBlinnLightingUniforms *uniforms = (VROBlinnLightingUniforms *)[_lightingUniformsBuffer contents];

    switch (light->getType()) {
        case VROLightType::Ambient:

            break;
            
        case VROLightType::Directional:
            
            break;
            
        case VROLightType::Omni:
            
            break;
            
        case VROLightType::Spot:
            
            break;
            
        default:
            break;
    }
}

void VROMaterialSubstrateMetal::bindPhongLighting(const std::shared_ptr<VROLight> &light) {
    VROPhongLightingUniforms *uniforms = (VROPhongLightingUniforms *)[_lightingUniformsBuffer contents];

    switch (light->getType()) {
        case VROLightType::Ambient:

            break;
            
        case VROLightType::Directional:
            
            break;
            
        case VROLightType::Omni:
            
            break;
            
        case VROLightType::Spot:
            
            break;
            
        default:
            break;
    }
}

void VROMaterialSubstrateMetal::bindLambertLighting(const std::shared_ptr<VROLight> &light) {
    VROLambertLightingUniforms *uniforms = (VROLambertLightingUniforms *)[_lightingUniformsBuffer contents];

    switch (light->getType()) {
        case VROLightType::Ambient:

            break;
            
        case VROLightType::Directional:
            
            break;
            
        case VROLightType::Omni:
            
            break;
            
        case VROLightType::Spot:
            
            break;
            
        default:
            break;
    }
}
