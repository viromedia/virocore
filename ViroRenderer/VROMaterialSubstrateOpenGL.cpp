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
    _lightingModel(material.getLightingModel()),
    _program(nullptr) {

    switch (material.getLightingModel()) {
        case VROLightingModel::Constant:
            loadConstantLighting(material, driver);
            break;
                
        case VROLightingModel::Blinn:
            loadBlinnLighting(material, driver);
            break;
                
        case VROLightingModel::Lambert:
            loadLambertLighting(material, driver);
            break;
                
        case VROLightingModel::Phong:
            loadPhongLighting(material, driver);
            break;
                
        default:
            break;
    }
        
    loadLightUniforms(_program);
    _program->hydrate();
        
    ALLOCATION_TRACKER_ADD(MaterialSubstrates, 1);
}
    
VROMaterialSubstrateOpenGL::~VROMaterialSubstrateOpenGL() {
    delete (_program);
    
    ALLOCATION_TRACKER_SUB(MaterialSubstrates, 1);
}

void VROMaterialSubstrateOpenGL::loadConstantLighting(const VROMaterial &material, const VRODriverOpenGL &driver) {
    VROMaterialVisual &diffuse = material.getDiffuse();
    
    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        _program = new VROShaderProgram("constant_vsh", "constant_c_fsh",
                                        ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
    }
    else if (diffuse.getContentsType() == VROContentsType::Texture2D) {
        _textures.push_back(diffuse.getContentsTexture());
        
        _program = new VROShaderProgram("constant_vsh", "constant_t_fsh",
                                        ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
        _program->addSampler("sampler");
    }
    else {
        _textures.push_back(diffuse.getContentsTexture());
        
        _program = new VROShaderProgram("constant_vsh", "constant_q_fsh",
                                        ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
        _program->addSampler("sampler");
    }
}

void VROMaterialSubstrateOpenGL::loadLambertLighting(const VROMaterial &material, const VRODriverOpenGL &driver) {
    VROMaterialVisual &diffuse = material.getDiffuse();
    VROMaterialVisual &reflective = material.getReflective();
    
    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            _program = new VROShaderProgram("lambert_vsh", "lambert_c_reflect_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program->addSampler("reflect_texture");
        }
        else {
            _program = new VROShaderProgram("lambert_vsh", "lambert_c_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
        }
    }
    else {
        _textures.push_back(diffuse.getContentsTexture());
        
        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            _program = new VROShaderProgram("lambert_vsh", "lambert_t_reflect_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program->addSampler("texture");
            _program->addSampler("reflect_texture");
        }
        else {
            _program = new VROShaderProgram("lambert_vsh", "lambert_t_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program->addSampler("texture");
        }
    }
}

void VROMaterialSubstrateOpenGL::loadPhongLighting(const VROMaterial &material, const VRODriverOpenGL &driver) {
    /*
     If there's no specular map, then we fall back to Lambert lighting.
     */
    VROMaterialVisual &specular = material.getSpecular();
    if (specular.getContentsType() != VROContentsType::Texture2D) {
        loadLambertLighting(material, driver);
        return;
    }
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    VROMaterialVisual &reflective = material.getReflective();
    
    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        _textures.push_back(specular.getContentsTexture());
        
        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            
            _program = new VROShaderProgram("phong_vsh", "phong_c_reflect_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program->addSampler("specular_texture");
            _program->addSampler("reflect_texture");
        }
        else {
            _program = new VROShaderProgram("phong_vsh", "phong_c_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program->addSampler("specular_texture");
        }
    }
    else {
        _textures.push_back(diffuse.getContentsTexture());
        _textures.push_back(specular.getContentsTexture());
        
        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            
            _program = new VROShaderProgram("phong_vsh", "phong_t_reflect_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program->addSampler("diffuse_texture");
            _program->addSampler("specular_texture");
            _program->addSampler("reflect_texture");
        }
        else {
            _program = new VROShaderProgram("phong_vsh", "phong_t_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program->addSampler("diffuse_texture");
            _program->addSampler("specular_texture");
        }
    }
}

void VROMaterialSubstrateOpenGL::loadBlinnLighting(const VROMaterial &material, const VRODriverOpenGL &driver) {
    /*
     If there's no specular map, then we fall back to Lambert lighting.
     */
    VROMaterialVisual &specular = material.getSpecular();
    if (specular.getContentsType() != VROContentsType::Texture2D) {
        loadLambertLighting(material, driver);
        return;
    }
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    VROMaterialVisual &reflective = material.getReflective();
    
    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        _textures.push_back(specular.getContentsTexture());
        
        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            
            _program = new VROShaderProgram("blinn_vsh", "blinn_c_reflect_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program->addSampler("specular_texture");
            _program->addSampler("reflect_texture");
        }
        else {
            _program = new VROShaderProgram("blinn_vsh", "blinn_c_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program->addSampler("specular_texture");
        }
    }
    else {
        _textures.push_back(diffuse.getContentsTexture());
        _textures.push_back(specular.getContentsTexture());
        
        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            
            _program = new VROShaderProgram("blinn_vsh", "blinn_t_reflect_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program->addSampler("diffuse_texture");
            _program->addSampler("specular_texture");
            _program->addSampler("reflect_texture");
        }
        else {
            _program = new VROShaderProgram("blinn_vsh", "blinn_t_fsh",
                                            ((int)VROShaderMask::Tex | (int)VROShaderMask::Norm));
            _program->addSampler("diffuse_texture");
            _program->addSampler("specular_texture");
        }
    }
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
    
    if (_material.getWritesToDepthBuffer()) {
        glDepthMask(GL_TRUE);
    }
    else {
        glDepthMask(GL_FALSE);
    }
    
    if (_material.getReadsFromDepthBuffer()) {
        glDepthFunc(GL_LEQUAL);
    }
    else {
        glDepthFunc(GL_ALWAYS);
    }
}

void VROMaterialSubstrateOpenGL::bindViewUniforms(VROMatrix4f transform, VROMatrix4f modelview,
                                                  VROMatrix4f projectionMatrix, VROVector3f cameraPosition) {
    
    _program->setUniformValueMat4(transform.invert().transpose(), "normal_matrix");
    _program->setUniformValueMat4(transform, "model_matrix");
    _program->setUniformValueMat4(modelview, "modelview_matrix");
    _program->setUniformValueMat4(projectionMatrix.multiply(modelview), "modelview_projection_matrix");
    _program->setUniformValueVec3(cameraPosition, "camera_position");
}

void VROMaterialSubstrateOpenGL::bindMaterialUniforms(VRORenderParameters &params, VROEyeType eye, int frame) {
    _program->setUniformValueVec4(_material.getDiffuse().getContentsColor(), "material_diffuse_surface_color");
    _program->setUniformValueFloat(_material.getDiffuse().getIntensity(), "material_diffuse_intensity");
    _program->setUniformValueFloat(_material.getTransparency() * params.opacities.top(), "material_alpha");
    _program->setUniformValueFloat(_material.getShininess(), "material_shininess");
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
