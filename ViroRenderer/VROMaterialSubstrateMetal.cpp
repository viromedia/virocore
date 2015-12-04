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

void VROMaterialSubstrateMetal::bind(const VROMatrix4f &transform, const VROMatrix4f &projection) {
    VROConstantLightingUniforms *uniforms = (VROConstantLightingUniforms *)[_uniformsBuffer contents];
    uniforms->normal_matrix = toMatrixFloat4x4(transform.transpose().invert());
    uniforms->modelview_projection_matrix = toMatrixFloat4x4(projection.multiply(transform));
}

void VROMaterialSubstrateMetal::loadConstantLighting(VROMaterial &material,
                                                     id <MTLLibrary> library, id <MTLDevice> device) {
    
    _vertexProgram   = [library newFunctionWithName:@"constant_lighting_vertex"];
    _fragmentProgram = [library newFunctionWithName:@"constant_lighting_fragment"];
    
    _uniformsBuffer = [device newBufferWithLength:sizeof(VROConstantLightingUniforms) options:0];
    _uniformsBuffer.label = @"VROConstantLightingUniformBuffer";
    
    VROConstantLightingUniforms *uniforms = (VROConstantLightingUniforms *)[_uniformsBuffer contents];

    VROMaterialVisual &ambient = material.getAmbient();
    uniforms->ambient_color = toVectorFloat4(ambient.getContentsColor());
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    uniforms->diffuse_color = toVectorFloat4(diffuse.getContentsColor());
}

void VROMaterialSubstrateMetal::loadBlinnLighting(VROMaterial &material,
                                                  id <MTLLibrary> library, id <MTLDevice> device) {
    
    _vertexProgram   = [library newFunctionWithName:@"blinn_lighting_vertex"];
    _fragmentProgram = [library newFunctionWithName:@"blinn_lighting_fragment"];
    
    _uniformsBuffer = [device newBufferWithLength:sizeof(VROBlinnLightingUniforms) options:0];
    _uniformsBuffer.label = @"VROBlinnLightingUniformBuffer";
    
    VROBlinnLightingUniforms *uniforms = (VROBlinnLightingUniforms *)[_uniformsBuffer contents];

    VROMaterialVisual &ambient = material.getAmbient();
    uniforms->ambient_color = toVectorFloat4(ambient.getContentsColor());
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    uniforms->diffuse_color = toVectorFloat4(diffuse.getContentsColor());
    
}

void VROMaterialSubstrateMetal::loadPhongLighting(VROMaterial &material,
                                                  id <MTLLibrary> library, id <MTLDevice> device) {
    
    _vertexProgram   = [library newFunctionWithName:@"phong_lighting_vertex"];
    _fragmentProgram = [library newFunctionWithName:@"phong_lighting_fragment"];
    
    _uniformsBuffer = [device newBufferWithLength:sizeof(VROPhongLightingUniforms) options:0];
    _uniformsBuffer.label = @"VROPhongLightingUniformBuffer";
    
    VROPhongLightingUniforms *uniforms = (VROPhongLightingUniforms *)[_uniformsBuffer contents];
    //TODO
}

void VROMaterialSubstrateMetal::loadLambertLighting(VROMaterial &material,
                                                    id <MTLLibrary> library, id <MTLDevice> device) {
    
    _vertexProgram   = [library newFunctionWithName:@"lambert_lighting_vertex"];
    _fragmentProgram = [library newFunctionWithName:@"lambert_lighting_fragment"];
    
    _uniformsBuffer = [device newBufferWithLength:sizeof(VROLambertLightingUniforms) options:0];
    _uniformsBuffer.label = @"VROLambertLightingUniformBuffer";
    
    VROLambertLightingUniforms *uniforms = (VROLambertLightingUniforms *)[_uniformsBuffer contents];
    //TODO
}