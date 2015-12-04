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

VROMaterialSubstrateMetal::VROMaterialSubstrateMetal(VROMaterial &material,
                                                     const VRORenderContextMetal &context) {
    

    id <MTLDevice> device = context.getDevice();
    id <MTLLibrary> library = context.getLibrary();
    
    _viewUniformsBuffer = [device newBufferWithLength:sizeof(VROViewUniforms) options:0];
    _viewUniformsBuffer.label = @"VROViewUniformBuffer";
    
    switch (material.getLightingModel()) {
        case VROLightingModel::Constant:
            loadConstantLighting(material, library, device);
            break;
            
        case VROLightingModel::Blinn:
            loadBlinnLighting(material, library, device);
            break;
            
        case VROLightingModel::Lambert:
            loadLambertLighting(material, library, device);
            break;
            
        case VROLightingModel::Phong:
            loadPhongLighting(material, library, device);
            break;
            
        default:
            break;
    }
}

VROMaterialSubstrateMetal::~VROMaterialSubstrateMetal() {
    
}

void VROMaterialSubstrateMetal::loadConstantLighting(VROMaterial &material,
                                                     id <MTLLibrary> library, id <MTLDevice> device) {
    
    _vertexProgram   = [library newFunctionWithName:@"constant_lighting_vertex"];
    _fragmentProgram = [library newFunctionWithName:@"constant_lighting_fragment"];
    
    _lightingUniformsBuffer = [device newBufferWithLength:sizeof(VROConstantLightingUniforms) options:0];
    _lightingUniformsBuffer.label = @"VROConstantLightingUniformBuffer";
    
    VROConstantLightingUniforms *uniforms = (VROConstantLightingUniforms *)[_lightingUniformsBuffer contents];

    VROMaterialVisual &ambient = material.getAmbient();
    uniforms->ambient_color = toVectorFloat4(ambient.getContentsColor());
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    uniforms->diffuse_color = toVectorFloat4(diffuse.getContentsColor());
}

void VROMaterialSubstrateMetal::loadBlinnLighting(VROMaterial &material,
                                                  id <MTLLibrary> library, id <MTLDevice> device) {
    
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
                                                  id <MTLLibrary> library, id <MTLDevice> device) {
    
    _vertexProgram   = [library newFunctionWithName:@"phong_lighting_vertex"];
    _fragmentProgram = [library newFunctionWithName:@"phong_lighting_fragment"];
    
    _lightingUniformsBuffer = [device newBufferWithLength:sizeof(VROPhongLightingUniforms) options:0];
    _lightingUniformsBuffer.label = @"VROPhongLightingUniformBuffer";
    
    VROPhongLightingUniforms *uniforms = (VROPhongLightingUniforms *)[_lightingUniformsBuffer contents];
    //TODO
}

void VROMaterialSubstrateMetal::loadLambertLighting(VROMaterial &material,
                                                    id <MTLLibrary> library, id <MTLDevice> device) {
    
    _vertexProgram   = [library newFunctionWithName:@"lambert_lighting_vertex"];
    _fragmentProgram = [library newFunctionWithName:@"lambert_lighting_fragment"];
    
    _lightingUniformsBuffer = [device newBufferWithLength:sizeof(VROLambertLightingUniforms) options:0];
    _lightingUniformsBuffer.label = @"VROLambertLightingUniformBuffer";
    
    VROLambertLightingUniforms *uniforms = (VROLambertLightingUniforms *)[_lightingUniformsBuffer contents];
    //TODO
}