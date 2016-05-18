//
//  VROMaterialSubstrateOpenGL.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROMaterialSubstrateOpenGL.h"
#include "VRODriverOpenGL.h"
#include "VROMaterial.h"
#include "VROShaderBuilder.h"
#include "VROShaderProgram.h"
#include "VROAllocationTracker.h"
#include "VROEye.h"
#include "VROLight.h"
#include "VRORenderParameters.h"
#include <sstream>

static const int kMaxLights = 4;

VROMaterialSubstrateOpenGL::VROMaterialSubstrateOpenGL(const VROMaterial &material, const VRODriverOpenGL &driver) :
    _material(material),
    _lightingModel(material.getLightingModel()) {

    switch (material.getLightingModel()) {
        case VROLightingModel::Constant:
            loadLambertLighting(material, driver);
            break;
                
        case VROLightingModel::Blinn:
            loadLambertLighting(material, driver);
            break;
                
        case VROLightingModel::Lambert:
            loadLambertLighting(material, driver);
            break;
                
        case VROLightingModel::Phong:
            loadLambertLighting(material, driver);
            break;
                
        default:
            break;
    }
        
    ALLOCATION_TRACKER_ADD(MaterialSubstrates, 1);
}
    
VROMaterialSubstrateOpenGL::~VROMaterialSubstrateOpenGL() {
    ALLOCATION_TRACKER_SUB(MaterialSubstrates, 1);
}

void VROMaterialSubstrateOpenGL::loadLambertLighting(const VROMaterial &material, const VRODriverOpenGL &driver) {
    VROMaterialVisual &diffuse = material.getDiffuse();
    VROMaterialVisual &reflective = material.getReflective();
    
    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            //_program = new VROShaderProgram("lambert_c_vsh", "lambert_c_reflect_fsh",
            //                                ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program = new VROShaderProgram("lambert_c_vsh", "lambert_c_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            
            _program->addSampler("reflect_texture");
        }
        else {
            _program = new VROShaderProgram("lambert_c_vsh", "lambert_c_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
        }
    }
    else {
        _textures.push_back(diffuse.getContentsTexture());
        
        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            //_program = new VROShaderProgram("lambert_c_vsh", "lambert_t_reflect_fsh",
            //                                ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program = new VROShaderProgram("lambert_c_vsh", "lambert_c_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program->addSampler("texture");
            _program->addSampler("reflect_texture");
        }
        else {
            _program = new VROShaderProgram("lambert_c_vsh", "lambert_t_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program->addSampler("texture");
        }
    }
    
    loadLightUniforms(_program);
    _program->hydrate();
}

void VROMaterialSubstrateOpenGL::loadLightUniforms(VROShaderProgram *program) {
    program->addUniform(VROShaderProperty::Int, 1, "lighting.num_lights");
    program->addUniform(VROShaderProperty::Vec3, 1, "lighting.ambient_light_color");
    
    for (int i = 0; i < kMaxLights; i++) {
        std::stringstream ss;
        ss << "lighting.lights[" << i << "].";
        
        std::string prefix = ss.str();
        program->addUniform(VROShaderProperty::Int, 1, prefix + "type");
        program->addUniform(VROShaderProperty::Vec3, 1, prefix + "position");
        program->addUniform(VROShaderProperty::Vec3, 1, prefix + "direction");
        program->addUniform(VROShaderProperty::Vec3, 1, prefix + "color");
        
        program->addUniform(VROShaderProperty::Float, 1, prefix + "attenuation_start_distance");
        program->addUniform(VROShaderProperty::Float, 1, prefix + "attenuation_end_distance");
        program->addUniform(VROShaderProperty::Float, 1, prefix + "attenuation_falloff_exp");
        program->addUniform(VROShaderProperty::Float, 1, prefix + "spot_inner_angle");
        program->addUniform(VROShaderProperty::Float, 1, prefix + "spot_outer_angle");
    }
    
    program->addUniform(VROShaderProperty::Mat4, 1, "normal_matrix");
    program->addUniform(VROShaderProperty::Mat4, 1, "model_matrix");
    program->addUniform(VROShaderProperty::Mat4, 1, "modelview_matrix");
    program->addUniform(VROShaderProperty::Mat4, 1, "modelview_projection_matrix");
    program->addUniform(VROShaderProperty::Vec3, 1, "camera_position");
    program->addUniform(VROShaderProperty::Vec3, 1, "ambient_light_color");
    program->addUniform(VROShaderProperty::Vec4, 1, "material_diffuse_surface_color");
    program->addUniform(VROShaderProperty::Float, 1, "material_diffuse_intensity");
    program->addUniform(VROShaderProperty::Float, 1, "material_alpha");
}

void VROMaterialSubstrateOpenGL::bindShader() {
    _program->bind();
}

void VROMaterialSubstrateOpenGL::bindViewUniforms(VROMatrix4f transform, VROMatrix4f modelview,
                                                  VROMatrix4f projectionMatrix, VROVector3f cameraPosition) {
    
    _program->setUniformValueMat4(transform.invert().transpose(), "normal_matrix");
    _program->setUniformValueMat4(transform, "model_matrix");
   // _program->setUniformValueMat4(modelview, "modelview_matrix");
    _program->setUniformValueMat4(projectionMatrix.multiply(modelview), "modelview_projection_matrix");
    _program->setUniformValueVec3(cameraPosition, "camera_position");
}

void VROMaterialSubstrateOpenGL::bindMaterialUniforms(VRORenderParameters &params, VROEyeType eye, int frame) {
    _program->setUniformValueVec4(_material.getDiffuse().getContentsColor(), "material_diffuse_surface_color");
    _program->setUniformValueFloat(_material.getDiffuse().getIntensity(), "material_diffuse_intensity");
    _program->setUniformValueFloat(_material.getTransparency() * params.opacities.top(), "material_alpha");
    //_program->setUniformValueFloat(_material.getShininess(), "material_shininess");
}

void VROMaterialSubstrateOpenGL::bindLightingUniforms(const std::vector<std::shared_ptr<VROLight>> &lights,
                                                      VROEyeType eye, int frame) {
   
    _program->setUniformValueInt((int)lights.size(), "lighting.num_lights");
    
    VROVector3f ambientLight;
    
    for (int i = 0; i < lights.size(); i++) {
        std::stringstream ss;
        ss << "lighting.lights[" << i << "].";
        std::string prefix = ss.str();
        
        const std::shared_ptr<VROLight> &light = lights[i];
    
        _program->setUniformValueInt((int) light->getType(), prefix + "type");
        _program->setUniformValueVec3(light->getTransformedPosition(), prefix + "position");
        _program->setUniformValueVec3(light->getDirection(), prefix + "direction");
        _program->setUniformValueVec3(light->getColor(), prefix + "color");
        _program->setUniformValueFloat(light->getAttenuationStartDistance(), prefix + "attenuation_start_distance");
        _program->setUniformValueFloat(light->getAttenuationEndDistance(), prefix + "attenuation_end_distance");
        _program->setUniformValueFloat(light->getAttenuationFalloffExponent(), prefix + "attenuation_falloff_exp");
        _program->setUniformValueFloat(light->getSpotInnerAngle(), prefix + "spot_inner_angle");
        _program->setUniformValueFloat(light->getSpotOuterAngle(), prefix + "spot_outer_angle");
        
        if (light->getType() == VROLightType::Ambient) {
            ambientLight += light->getColor();
        }
    }
    
    _program->setUniformValueVec3(ambientLight, "lighting.ambient_light_color");
}
