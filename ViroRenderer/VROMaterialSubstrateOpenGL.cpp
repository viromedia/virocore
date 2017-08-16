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
#include "VROSortKey.h"
#include "VROBoneUBO.h"
#include "VROInstancedUBO.h"
#include "VROShaderFactory.h"
#include <sstream>

#pragma mark - Loading Materials

void VROMaterialSubstrateOpenGL::hydrateProgram(std::shared_ptr<VRODriverOpenGL> &driver) {
    _program->hydrate();
}

VROMaterialSubstrateOpenGL::VROMaterialSubstrateOpenGL(VROMaterial &material, std::shared_ptr<VRODriverOpenGL> &driver) :
    _material(material),
    _lightingModel(material.getLightingModel()),
    _program(nullptr),
    _diffuseSurfaceColorUniform(nullptr),
    _diffuseIntensityUniform(nullptr),
    _alphaUniform(nullptr),
    _shininessUniform(nullptr),
    _normalMatrixUniform(nullptr),
    _modelMatrixUniform(nullptr),
    _viewMatrixUniform(nullptr),
    _projectionMatrixUniform(nullptr),
    _cameraPositionUniform(nullptr),
    _eyeTypeUniform(nullptr),
    _shadowViewMatrixUniform(nullptr),
    _shadowProjectionMatrixUniform(nullptr) {

    // TODO VIRO-1501 Move this to where lights are bound
    const std::vector<std::shared_ptr<VROLight>> lights;
    _program = driver->getShaderFactory()->getShader(material, lights, driver);
    if (!_program->isHydrated()) {
        hydrateProgram(driver);
    }
    loadUniforms();
    loadSamplers();
        
    ALLOCATION_TRACKER_ADD(MaterialSubstrates, 1);
}
    
VROMaterialSubstrateOpenGL::~VROMaterialSubstrateOpenGL() {
    ALLOCATION_TRACKER_SUB(MaterialSubstrates, 1);
}

void VROMaterialSubstrateOpenGL::loadUniforms() {
    _diffuseSurfaceColorUniform = _program->getUniform("material_diffuse_surface_color");
    _diffuseIntensityUniform = _program->getUniform("material_diffuse_intensity");
    _alphaUniform = _program->getUniform("material_alpha");
    _shininessUniform = _program->getUniform("material_shininess");
    
    _normalMatrixUniform = _program->getUniform("normal_matrix");
    _modelMatrixUniform = _program->getUniform("model_matrix");
    _projectionMatrixUniform = _program->getUniform("projection_matrix");
    _viewMatrixUniform = _program->getUniform("view_matrix");
    _cameraPositionUniform = _program->getUniform("camera_position");
    _eyeTypeUniform = _program->getUniform("eye_type");
    _shadowViewMatrixUniform = _program->getUniform("shadow_view_matrix");
    _shadowProjectionMatrixUniform = _program->getUniform("shadow_projection_matrix");
    
    for (const std::shared_ptr<VROShaderModifier> &modifier : _program->getModifiers()) {
        std::vector<std::string> uniformNames = modifier->getUniforms();
        
        for (std::string &uniformName : uniformNames) {
            VROUniform *uniform = _program->getUniform(uniformName);
            passert_msg (uniform != nullptr, "Failed to find shader modifier uniform '%s' in program!",
                         uniformName.c_str());
            
            _shaderModifierUniforms.push_back(uniform);
        }
    }
}

void VROMaterialSubstrateOpenGL::loadSamplers() {
    const std::vector<std::string> &samplers = _program->getSamplers();
    for (const std::string &sampler : samplers) {
        if (sampler == "diffuse_texture" || sampler == "diffuse_texture_y") {
            _textures.push_back(_material.getDiffuse().getTexture());
        }
        else if (sampler == "specular_texture") {
            _textures.push_back(_material.getSpecular().getTexture());
        }
        else if (sampler == "normal_texture") {
            _textures.push_back(_material.getNormal().getTexture());
        }
        else if (sampler == "reflect_texture") {
            _textures.push_back(_material.getReflective().getTexture());
        }
    }
}

#pragma mark - Binding Materials

void VROMaterialSubstrateOpenGL::bindProperties() {
    bindMaterialUniforms();
}

void VROMaterialSubstrateOpenGL::bindGeometry(float opacity, const VROGeometry &geometry){
    bindGeometryUniforms(opacity, geometry);
}

void VROMaterialSubstrateOpenGL::bindShader() {
    _program->bind();
}

void VROMaterialSubstrateOpenGL::bindLights(int lightsHash,
                                            const std::vector<std::shared_ptr<VROLight>> &lights,
                                            const VRORenderContext &context,
                                            std::shared_ptr<VRODriver> &driver) {
    
    if (lights.empty()) {
        VROLightingUBO::unbind(_program);
        _lightingUBO.reset();
        return;
    }
    
    VRODriverOpenGL &glDriver = (VRODriverOpenGL &)(*driver.get());
    for (const std::shared_ptr<VROLight> &light : lights) {
        light->propagateUpdates();
    }

    if (!_lightingUBO || _lightingUBO->getHash() != lightsHash) {
        _lightingUBO = glDriver.getLightingUBO(lightsHash);
        if (!_lightingUBO) {
            _lightingUBO = glDriver.createLightingUBO(lightsHash, lights);
        }
    }
    
    _lightingUBO->bind(_program);
}

void VROMaterialSubstrateOpenGL::bindView(VROMatrix4f modelMatrix, VROMatrix4f viewMatrix,
                                          VROMatrix4f projectionMatrix, VROMatrix4f normalMatrix,
                                          VROVector3f cameraPosition, VROEyeType eyeType,
                                          VROMatrix4f shadowViewMatrix, VROMatrix4f shadowProjectionMatrix) {
    if (_normalMatrixUniform != nullptr) {
        _normalMatrixUniform->setMat4(normalMatrix);
    }
    if (_modelMatrixUniform != nullptr) {
        _modelMatrixUniform->setMat4(modelMatrix);
    }
    if (_projectionMatrixUniform != nullptr) {
        _projectionMatrixUniform->setMat4(projectionMatrix);
    }
    if (_viewMatrixUniform != nullptr) {
        _viewMatrixUniform->setMat4(viewMatrix);
    }
    if (_cameraPositionUniform != nullptr) {
        _cameraPositionUniform->setVec3(cameraPosition);
    }
    if (_eyeTypeUniform != nullptr){
        _eyeTypeUniform->setInt(static_cast<int>(eyeType));
    }
    if (_shadowViewMatrixUniform != nullptr) {
        _shadowViewMatrixUniform->setMat4(shadowViewMatrix);
    }
    if (_shadowProjectionMatrixUniform != nullptr) {
        _shadowProjectionMatrixUniform->setMat4(shadowProjectionMatrix);
    }
}

void VROMaterialSubstrateOpenGL::bindMaterialUniforms() {
    if (_diffuseSurfaceColorUniform != nullptr) {
        _diffuseSurfaceColorUniform->setVec4(_material.getDiffuse().getColor());
    }
    if (_diffuseIntensityUniform != nullptr) {
        _diffuseIntensityUniform->setFloat(_material.getDiffuse().getIntensity());
    }
    if (_shininessUniform != nullptr) {
        _shininessUniform->setFloat(_material.getShininess());
    }
}

void VROMaterialSubstrateOpenGL::bindGeometryUniforms(float opacity, const VROGeometry &geometry) {
    if (_alphaUniform != nullptr) {
        _alphaUniform->setFloat(_material.getTransparency() * opacity);
    }
    for (VROUniform *uniform : _shaderModifierUniforms) {
        uniform->set(nullptr, &geometry);
    }
}

void VROMaterialSubstrateOpenGL::bindBoneUBO(const std::unique_ptr<VROBoneUBO> &boneUBO) {
    boneUBO->bind(_program);
}

void VROMaterialSubstrateOpenGL::bindInstanceUBO(const std::shared_ptr<VROInstancedUBO> &instanceUBO) {
    instanceUBO->bind(_program);
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
