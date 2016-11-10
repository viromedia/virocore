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
#include "VROShaderProgram.h"
#include "VROAllocationTracker.h"
#include "VROEye.h"
#include "VROLight.h"
#include "VRORenderParameters.h"
#include "VROSortKey.h"
#include <sstream>

void VROMaterialSubstrateOpenGL::hydrateProgram(VRODriverOpenGL &driver) {
    _program->hydrate();
}

VROMaterialSubstrateOpenGL::VROMaterialSubstrateOpenGL(const VROMaterial &material, VRODriverOpenGL &driver) :
    _material(material),
    _lightingModel(material.getLightingModel()),
    _program(nullptr),
    _diffuseSurfaceColorUniform(nullptr),
    _diffuseIntensityUniform(nullptr),
    _alphaUniform(nullptr),
    _shininessUniform(nullptr),
    _normalMatrixUniform(nullptr),
    _modelMatrixUniform(nullptr),
    _modelViewMatrixUniform(nullptr),
    _modelViewProjectionMatrixUniform(nullptr),
    _cameraPositionUniform(nullptr) {

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

void VROMaterialSubstrateOpenGL::loadConstantLighting(const VROMaterial &material, VRODriverOpenGL &driver) {
    VROMaterialVisual &diffuse = material.getDiffuse();
    
    std::string vertexShader = "standard_vsh";
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
    
    _program = driver.getPooledShader(vertexShader, fragmentShader, samplers,
                                      material.getShaderModifiers());
    if (!_program->isHydrated()) {
        addUniforms();
        hydrateProgram(driver);
    }
    else {
        loadUniforms();
    }
}

void VROMaterialSubstrateOpenGL::loadLambertLighting(const VROMaterial &material, VRODriverOpenGL &driver) {
    std::string vertexShader = "standard_vsh";
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
    
    _program = driver.getPooledShader(vertexShader, fragmentShader, samplers,
                                      material.getShaderModifiers());
    if (!_program->isHydrated()) {
        addUniforms();
        hydrateProgram(driver);
    }
    else {
        loadUniforms();
    }
}

void VROMaterialSubstrateOpenGL::loadPhongLighting(const VROMaterial &material, VRODriverOpenGL &driver) {
    std::string vertexShader = "standard_vsh";
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

    _program = driver.getPooledShader(vertexShader, fragmentShader, samplers,
                                      material.getShaderModifiers());
    if (!_program->isHydrated()) {
        addUniforms();
        _shininessUniform = _program->addUniform(VROShaderProperty::Float, 1, "material_shininess");
        hydrateProgram(driver);
    }
    else {
        _shininessUniform = _program->getUniform("material_shininess");
        loadUniforms();
    }
}

void VROMaterialSubstrateOpenGL::loadBlinnLighting(const VROMaterial &material, VRODriverOpenGL &driver) {
    std::string vertexShader = "standard_vsh";
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
    
    _program = driver.getPooledShader(vertexShader, fragmentShader, samplers,
                                      material.getShaderModifiers());
    if (!_program->isHydrated()) {
        addUniforms();
        _shininessUniform = _program->addUniform(VROShaderProperty::Float, 1, "material_shininess");
        hydrateProgram(driver);
    }
    else {
        _shininessUniform = _program->getUniform("material_shininess");
        loadUniforms();
    }
}

void VROMaterialSubstrateOpenGL::addUniforms() {
    _normalMatrixUniform = _program->addUniform(VROShaderProperty::Mat4, 1, "normal_matrix");
    _modelMatrixUniform = _program->addUniform(VROShaderProperty::Mat4, 1, "model_matrix");
    _modelViewMatrixUniform = _program->addUniform(VROShaderProperty::Mat4, 1, "modelview_matrix");
    _modelViewProjectionMatrixUniform = _program->addUniform(VROShaderProperty::Mat4, 1, "modelview_projection_matrix");
    _cameraPositionUniform = _program->addUniform(VROShaderProperty::Vec3, 1, "camera_position");
    
    _diffuseSurfaceColorUniform = _program->addUniform(VROShaderProperty::Vec4, 1, "material_diffuse_surface_color");
    _diffuseIntensityUniform = _program->addUniform(VROShaderProperty::Float, 1, "material_diffuse_intensity");
    _alphaUniform = _program->addUniform(VROShaderProperty::Float, 1, "material_alpha");
    
    for (const std::shared_ptr<VROShaderModifier> &modifier : _material.getShaderModifiers()) {
        std::vector<std::string> uniformNames = modifier->getUniforms();
        
        for (std::string &uniformName : uniformNames) {
            VROUniform *uniform = _program->getUniform(uniformName);
            _shaderModifierUniforms.push_back(uniform);
        }
    }
}

void VROMaterialSubstrateOpenGL::loadUniforms() {
    _diffuseSurfaceColorUniform = _program->getUniform("material_diffuse_surface_color");
    _diffuseIntensityUniform = _program->getUniform("material_diffuse_intensity");
    _alphaUniform = _program->getUniform("material_alpha");
    
    _normalMatrixUniform = _program->getUniform("normal_matrix");
    _modelMatrixUniform = _program->getUniform("model_matrix");
    _modelViewMatrixUniform = _program->getUniform("modelview_matrix");
    _modelViewProjectionMatrixUniform = _program->getUniform("modelview_projection_matrix");
    _cameraPositionUniform = _program->getUniform("camera_position");
    
    for (const std::shared_ptr<VROShaderModifier> &modifier : _material.getShaderModifiers()) {
        std::vector<std::string> uniformNames = modifier->getUniforms();
        
        for (std::string &uniformName : uniformNames) {
            VROUniform *uniform = _program->getUniform(uniformName);
            _shaderModifierUniforms.push_back(uniform);
        }
    }
}

void VROMaterialSubstrateOpenGL::bindShader() {
    _program->bind();
}

void VROMaterialSubstrateOpenGL::bindLights(int lightsHash,
                                            const std::vector<std::shared_ptr<VROLight>> &lights,
                                            const VRORenderContext &context,
                                            VRODriver &driver) {
    
    VRODriverOpenGL &glDriver = (VRODriverOpenGL &)driver;
    
    std::shared_ptr<VROLightingUBO> lightingUBO = glDriver.getLightingUBO(lightsHash);
    if (!lightingUBO) {
        lightingUBO = glDriver.createLightingUBO(lightsHash);
        lightingUBO->writeLights(lights);
    }
    else {
        // If any light was updated, rewrite the lights to the UBO
        for (const std::shared_ptr<VROLight> &light : lights) {
            if (light->isUpdated()) {
                lightingUBO->writeLights(lights);
                break;
            }
        }
    }
    
    lightingUBO->bind(_program);
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

void VROMaterialSubstrateOpenGL::bindCullingSettings() {
    VROCullMode cullMode = _material.getCullMode();
    
    if (cullMode == VROCullMode::None) {
        glDisable(GL_CULL_FACE);
    }
    else if (cullMode == VROCullMode::Back) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    else if (cullMode == VROCullMode::Front) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
    }
}

void VROMaterialSubstrateOpenGL::bindViewUniforms(VROMatrix4f transform, VROMatrix4f modelview,
                                                  VROMatrix4f projectionMatrix, VROMatrix4f normalMatrix,
                                                  VROVector3f cameraPosition) {
    
    if (_normalMatrixUniform != nullptr) {
        _normalMatrixUniform->setMat4(normalMatrix);
    }
    if (_modelMatrixUniform != nullptr) {
        _modelMatrixUniform->setMat4(transform);
    }
    if (_modelViewMatrixUniform != nullptr) {
        _modelViewMatrixUniform->setMat4(modelview);
    }
    if (_modelViewProjectionMatrixUniform != nullptr) {
        _modelViewProjectionMatrixUniform->setMat4(projectionMatrix.multiply(modelview));
    }
    if (_cameraPositionUniform != nullptr) {
        _cameraPositionUniform->setVec3(cameraPosition);
    }
}

void VROMaterialSubstrateOpenGL::bindMaterialUniforms(float opacity) {
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
    
    for (VROUniform *uniform : _shaderModifierUniforms) {
        uniform->set(nullptr);
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
