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
#include "VROSortKey.h"
#include <sstream>

static const int kMaxLights = 4;
static std::map<std::string, std::shared_ptr<VROShaderProgram>> _sharedPrograms;

VROMaterialSubstrateOpenGL::VROMaterialSubstrateOpenGL(const VROMaterial &material, const VRODriverOpenGL &driver) :
    _material(material),
    _lightingModel(material.getLightingModel()),
    _program(nullptr),
    _diffuseSurfaceColorUniform(nullptr),
    _diffuseIntensityUniform(nullptr),
    _alphaUniform(nullptr),
    _shininessUniform(nullptr) {

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
        
    ALLOCATION_TRACKER_ADD(MaterialSubstrates, 1);
}
    
VROMaterialSubstrateOpenGL::~VROMaterialSubstrateOpenGL() {
    ALLOCATION_TRACKER_SUB(MaterialSubstrates, 1);
}

void VROMaterialSubstrateOpenGL::loadConstantLighting(const VROMaterial &material, const VRODriverOpenGL &driver) {
    VROMaterialVisual &diffuse = material.getDiffuse();
    
    std::string vertexShader = "constant_vsh";
    std::string fragmentShader;
    
    std::vector<std::string> samplers;
    
    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        fragmentShader = "constant_c_fsh";
    }
    else if (diffuse.getContentsType() == VROContentsType::Texture2D) {
        _textures.push_back(diffuse.getContentsTexture());
        samplers.push_back("sampler");

        fragmentShader = "constant_t_fsh";
    }
    else {
        _textures.push_back(diffuse.getContentsTexture());
        samplers.push_back("sampler");

        fragmentShader = "constant_q_fsh";
    }
    
    _program = getPooledShader(vertexShader, fragmentShader, samplers);
    if (!_program->isHydrated()) {
        addUniforms();
        _program->hydrate();
    }
    else {
        loadUniforms();
    }
}

void VROMaterialSubstrateOpenGL::loadLambertLighting(const VROMaterial &material, const VRODriverOpenGL &driver) {
    std::string vertexShader = "lambert_vsh";
    std::string fragmentShader;
    
    std::vector<std::string> samplers;
    
    VROMaterialVisual &diffuse = material.getDiffuse();
    VROMaterialVisual &reflective = material.getReflective();
    
    if (diffuse.getContentsType() == VROContentsType::Fixed) {
        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            samplers.push_back("reflect_texture");

            fragmentShader = "lambert_c_reflect_fsh";
        }
        else {
            fragmentShader = "lambert_c_fsh";
        }
    }
    else {
        _textures.push_back(diffuse.getContentsTexture());
        samplers.push_back("texture");
        
        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            samplers.push_back("reflect_texture");
            
            fragmentShader = "lambert_t_reflect_fsh";
        }
        else {
            fragmentShader = "lambert_t_fsh";
        }
    }
    
    _program = getPooledShader(vertexShader, fragmentShader, samplers);
    if (!_program->isHydrated()) {
        addUniforms();
        _program->hydrate();
    }
    else {
        loadUniforms();
    }
}

void VROMaterialSubstrateOpenGL::loadPhongLighting(const VROMaterial &material, const VRODriverOpenGL &driver) {
    std::string vertexShader = "phong_vsh";
    std::string fragmentShader;
    
    std::vector<std::string> samplers;
    
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
        samplers.push_back("specular_texture");
        
        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            samplers.push_back("reflect_texture");
            
            fragmentShader = "phong_c_reflect_fsh";
        }
        else {
            fragmentShader = "phong_c_fsh";
        }
    }
    else {
        _textures.push_back(diffuse.getContentsTexture());
        _textures.push_back(specular.getContentsTexture());
        
        samplers.push_back("diffuse_texture");
        samplers.push_back("specular_texture");
        
        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            samplers.push_back("reflect_texture");

            fragmentShader = "phong_t_reflect_fsh";
        }
        else {
            fragmentShader = "phong_t_fsh";
        }
    }
    
    _program = getPooledShader(vertexShader, fragmentShader, samplers);
    if (!_program->isHydrated()) {
        addUniforms();
        _shininessUniform = _program->addUniform(VROShaderProperty::Float, 1, "material_shininess");
        _program->hydrate();
    }
    else {
        _shininessUniform = _program->getUniform("material_shininess");
        loadUniforms();
    }
}

void VROMaterialSubstrateOpenGL::loadBlinnLighting(const VROMaterial &material, const VRODriverOpenGL &driver) {
    std::string vertexShader = "blinn_vsh";
    std::string fragmentShader;
    
    std::vector<std::string> samplers;
    
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
        samplers.push_back("specular_texture");

        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            samplers.push_back("reflect_texture");
            
            fragmentShader = "blinn_c_reflect_fsh";
        }
        else {
            fragmentShader = "blinn_c_fsh";
        }
    }
    else {
        _textures.push_back(diffuse.getContentsTexture());
        _textures.push_back(specular.getContentsTexture());
        
        samplers.push_back("diffuse_texture");
        samplers.push_back("specular_texture");
        
        if (reflective.getContentsType() == VROContentsType::TextureCube) {
            _textures.push_back(reflective.getContentsTexture());
            samplers.push_back("reflect_texture");

            fragmentShader = "blinn_t_reflect_fsh";
        }
        else {
            fragmentShader = "blinn_t_fsh";
        }
    }
    
    _program = getPooledShader(vertexShader, fragmentShader, samplers);
    if (!_program->isHydrated()) {
        addUniforms();
        _shininessUniform = _program->addUniform(VROShaderProperty::Float, 1, "material_shininess");
        _program->hydrate();
    }
    else {
        _shininessUniform = _program->getUniform("material_shininess");
        loadUniforms();
    }
}

void VROMaterialSubstrateOpenGL::addUniforms() {
    _program->addUniform(VROShaderProperty::Int, 1, "lighting.num_lights");
    _program->addUniform(VROShaderProperty::Vec3, 1, "lighting.ambient_light_color");
    
    for (int i = 0; i < kMaxLights; i++) {
        std::stringstream ss;
        ss << "lighting.lights[" << i << "].";
        
        std::string prefix = ss.str();
        _program->addUniform(VROShaderProperty::Int, 1, prefix + "type");
        
        _program->addUniform(VROShaderProperty::Vec3, 1, prefix + "position");
        _program->addUniform(VROShaderProperty::Vec3, 1, prefix + "direction");
        _program->addUniform(VROShaderProperty::Vec3, 1, prefix + "color");
        
        _program->addUniform(VROShaderProperty::Float, 1, prefix + "attenuation_start_distance");
        _program->addUniform(VROShaderProperty::Float, 1, prefix + "attenuation_end_distance");
        _program->addUniform(VROShaderProperty::Float, 1, prefix + "attenuation_falloff_exp");
        _program->addUniform(VROShaderProperty::Float, 1, prefix + "spot_inner_angle");
        _program->addUniform(VROShaderProperty::Float, 1, prefix + "spot_outer_angle");
    }
    
    _program->addUniform(VROShaderProperty::Mat4, 1, "normal_matrix");
    _program->addUniform(VROShaderProperty::Mat4, 1, "model_matrix");
    _program->addUniform(VROShaderProperty::Mat4, 1, "modelview_matrix");
    _program->addUniform(VROShaderProperty::Mat4, 1, "modelview_projection_matrix");
    _program->addUniform(VROShaderProperty::Vec3, 1, "camera_position");
    _program->addUniform(VROShaderProperty::Vec3, 1, "ambient_light_color");
    
    _diffuseSurfaceColorUniform = _program->addUniform(VROShaderProperty::Vec4, 1, "material_diffuse_surface_color");
    _diffuseIntensityUniform = _program->addUniform(VROShaderProperty::Float, 1, "material_diffuse_intensity");
    _alphaUniform = _program->addUniform(VROShaderProperty::Float, 1, "material_alpha");
}

void VROMaterialSubstrateOpenGL::loadUniforms() {
    _diffuseSurfaceColorUniform = _program->getUniform("material_diffuse_surface_color");
    _diffuseIntensityUniform = _program->getUniform("material_diffuse_intensity");
    _alphaUniform = _program->getUniform("material_alpha");
}

void VROMaterialSubstrateOpenGL::bindShader() {
    _program->bind();
}

void VROMaterialSubstrateOpenGL::bindDepthSettings() {
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

void VROMaterialSubstrateOpenGL::bindMaterialUniforms(float opacity, VROEyeType eye, int frame) {
    if (_diffuseSurfaceColorUniform != nullptr) {
        _diffuseSurfaceColorUniform->setVec4(_material.getDiffuse().getContentsColor());
    }
    if (_diffuseIntensityUniform != nullptr) {
        _diffuseIntensityUniform->setFloat(_material.getDiffuse().getIntensity());
    }
    if (_alphaUniform != nullptr) {
        _alphaUniform->setFloat(_material.getTransparency() * opacity);
    }
    if (_shininessUniform != nullptr) {
        _shininessUniform->setFloat(_material.getShininess());
    }
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

std::shared_ptr<VROShaderProgram> VROMaterialSubstrateOpenGL::getPooledShader(std::string vertexShader,
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

void VROMaterialSubstrateOpenGL::updateSortKey(VROSortKey &key) const {
    key.shader = _program->getShaderId();
    key.textures = hashTextures(_textures);
}

uint32_t VROMaterialSubstrateOpenGL::hashTextures(const std::vector<std::shared_ptr<VROTexture>> &textures) const {
    uint32_t h = 0;
    for (const std::shared_ptr<VROTexture> &texture : textures) {
        h = 31 * h + texture->getTextureId();
    }
    return h;
}
