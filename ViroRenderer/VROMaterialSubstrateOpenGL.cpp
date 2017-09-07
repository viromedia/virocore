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

VROMaterialSubstrateOpenGL::VROMaterialSubstrateOpenGL(VROMaterial &material, std::shared_ptr<VRODriverOpenGL> &driver) :
    _material(material),
    _activeBinding(nullptr) {

    _materialShaderCapabilities = driver->getShaderFactory()->deriveMaterialCapabilitiesKey(material);
    ALLOCATION_TRACKER_ADD(MaterialSubstrates, 1);
}
    
VROMaterialSubstrateOpenGL::~VROMaterialSubstrateOpenGL() {
    ALLOCATION_TRACKER_SUB(MaterialSubstrates, 1);
}

#pragma mark - Binding Materials

void VROMaterialSubstrateOpenGL::bindProperties() {
    passert(_activeBinding != nullptr);
    _activeBinding->bindMaterialUniforms(_material);
}

void VROMaterialSubstrateOpenGL::bindGeometry(float opacity, const VROGeometry &geometry){
    passert(_activeBinding != nullptr);
    _activeBinding->bindGeometryUniforms(opacity, geometry, _material);
}

void VROMaterialSubstrateOpenGL::bindShader(int lightsHash,
                                            const std::vector<std::shared_ptr<VROLight>> &lights,
                                            std::shared_ptr<VRODriver> &driver) {
    
    _activeBinding = getShaderBindingForLights(lights, driver);
    driver->bindShader(_activeBinding->getProgram());

    if (lights.empty()) {
        VROLightingUBO::unbind(_activeBinding->getProgram());
        _lightingUBO.reset();
        return;
    }
    
    VRODriverOpenGL &glDriver = (VRODriverOpenGL &)(*driver.get());
    for (const std::shared_ptr<VROLight> &light : lights) {
        light->propagateFragmentUpdates();
        light->propagateVertexUpdates();
    }

    if (!_lightingUBO || _lightingUBO->getHash() != lightsHash) {
        _lightingUBO = glDriver.getLightingUBO(lightsHash);
        if (!_lightingUBO) {
            _lightingUBO = glDriver.createLightingUBO(lightsHash, lights);
        }
    }
    _lightingUBO->bind(_activeBinding->getProgram());
}

void VROMaterialSubstrateOpenGL::bindView(VROMatrix4f modelMatrix, VROMatrix4f viewMatrix,
                                          VROMatrix4f projectionMatrix, VROMatrix4f normalMatrix,
                                          VROVector3f cameraPosition, VROEyeType eyeType) {
    passert(_activeBinding != nullptr);
    _activeBinding->bindViewUniforms(modelMatrix, viewMatrix, projectionMatrix, normalMatrix,
                                     cameraPosition, eyeType);
}

void VROMaterialSubstrateOpenGL::bindBoneUBO(const std::unique_ptr<VROBoneUBO> &boneUBO) {
    passert(_activeBinding != nullptr);
    boneUBO->bind(_activeBinding->getProgram());
}

void VROMaterialSubstrateOpenGL::bindInstanceUBO(const std::shared_ptr<VROInstancedUBO> &instanceUBO) {
    passert(_activeBinding != nullptr);
    instanceUBO->bind(_activeBinding->getProgram());
}

#pragma mark - Updating Sort Key and Textures

void VROMaterialSubstrateOpenGL::updateTextures() {
    for (auto &kv : _shaderBindings) {
        kv.second->loadTextures();
    }
}

void VROMaterialSubstrateOpenGL::updateSortKey(VROSortKey &key, const std::vector<std::shared_ptr<VROLight>> &lights,
                                               std::shared_ptr<VRODriver> driver) {
    VROMaterialShaderBinding *binding = getShaderBindingForLights(lights, driver);
    passert (binding != nullptr);
    
    key.shader = binding->getProgram()->getShaderId();
    key.textures = hashTextures(binding->getTextures());
}

VROMaterialShaderBinding *VROMaterialSubstrateOpenGL::getShaderBindingForLights(const std::vector<std::shared_ptr<VROLight>> &lights,
                                                                                std::shared_ptr<VRODriver> driver) {
    std::shared_ptr<VRODriverOpenGL> driverGL = std::dynamic_pointer_cast<VRODriverOpenGL>(driver);
    VROLightingShaderCapabilities capabilities = VROShaderFactory::deriveLightingCapabilitiesKey(lights);
    
    // Optimized path: check the active binding
    if (_activeBinding != nullptr && _activeBinding->lightingShaderCapabilities == capabilities) {
        return _activeBinding;
    }
    
    // Next check our installed bindings
    auto it = _shaderBindings.find(capabilities);
    if (it != _shaderBindings.end()) {
        return it->second.get();
    }
    
    // Finally, check the shader factory, which will create a new shader if necessary
    std::shared_ptr<VROShaderProgram> shader = driverGL->getShaderFactory()->getShader(_materialShaderCapabilities, capabilities,
                                                                                       _material.getShaderModifiers(), driverGL);
    if (!shader->isHydrated()) {
        shader->hydrate();
    }
    VROMaterialShaderBinding *binding = new VROMaterialShaderBinding(shader, capabilities, _material);
    _shaderBindings[capabilities] = std::unique_ptr<VROMaterialShaderBinding>(binding);
    return binding;
}

uint32_t VROMaterialSubstrateOpenGL::hashTextures(const std::vector<std::shared_ptr<VROTexture>> &textures) const {
    uint32_t h = 0;
    for (const std::shared_ptr<VROTexture> &texture : textures) {
        h = 31 * h + texture->getTextureId();
    }
    return h;
}

#pragma mark - Material <-> Shader Binding

VROMaterialShaderBinding::VROMaterialShaderBinding(std::shared_ptr<VROShaderProgram> program,
                                                   VROLightingShaderCapabilities capabilities,
                                                   const VROMaterial &material) :
    _program(program),
    _material(material),
    lightingShaderCapabilities(capabilities),
    _diffuseSurfaceColorUniform(nullptr),
    _diffuseIntensityUniform(nullptr),
    _alphaUniform(nullptr),
    _shininessUniform(nullptr),
    _normalMatrixUniform(nullptr),
    _modelMatrixUniform(nullptr),
    _viewMatrixUniform(nullptr),
    _projectionMatrixUniform(nullptr),
    _cameraPositionUniform(nullptr),
    _eyeTypeUniform(nullptr) {
    
    loadUniforms();
    loadTextures();
}

VROMaterialShaderBinding::~VROMaterialShaderBinding() {
    
}

void VROMaterialShaderBinding::loadUniforms() {
    std::shared_ptr<VROShaderProgram> program = _program;
    
    _diffuseSurfaceColorUniform = program->getUniform("material_diffuse_surface_color");
    _diffuseIntensityUniform = program->getUniform("material_diffuse_intensity");
    _alphaUniform = program->getUniform("material_alpha");
    _shininessUniform = program->getUniform("material_shininess");
    
    _normalMatrixUniform = program->getUniform("normal_matrix");
    _modelMatrixUniform = program->getUniform("model_matrix");
    _projectionMatrixUniform = program->getUniform("projection_matrix");
    _viewMatrixUniform = program->getUniform("view_matrix");
    _cameraPositionUniform = program->getUniform("camera_position");
    _eyeTypeUniform = program->getUniform("eye_type");
    
    for (const std::shared_ptr<VROShaderModifier> &modifier : program->getModifiers()) {
        std::vector<std::string> uniformNames = modifier->getUniforms();
        
        for (std::string &uniformName : uniformNames) {
            VROUniform *uniform = program->getUniform(uniformName);
            passert_msg (uniform != nullptr, "Failed to find shader modifier uniform '%s' in program!",
                         uniformName.c_str());
            
            _shaderModifierUniforms.push_back(uniform);
        }
    }
}

void VROMaterialShaderBinding::loadTextures() {
    _textures.clear();

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

void VROMaterialShaderBinding::bindViewUniforms(VROMatrix4f &modelMatrix, VROMatrix4f &viewMatrix,
                                                VROMatrix4f &projectionMatrix, VROMatrix4f &normalMatrix,
                                                VROVector3f &cameraPosition, VROEyeType &eyeType) {
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
}

void VROMaterialShaderBinding::bindMaterialUniforms(const VROMaterial &material) {
    if (_diffuseSurfaceColorUniform != nullptr) {
        _diffuseSurfaceColorUniform->setVec4(material.getDiffuse().getColor());
    }
    if (_diffuseIntensityUniform != nullptr) {
        _diffuseIntensityUniform->setFloat(material.getDiffuse().getIntensity());
    }
    if (_shininessUniform != nullptr) {
        _shininessUniform->setFloat(material.getShininess());
    }
}

void VROMaterialShaderBinding::bindGeometryUniforms(float opacity, const VROGeometry &geometry, const VROMaterial &material) {
    if (_alphaUniform != nullptr) {
        _alphaUniform->setFloat(material.getTransparency() * opacity);
    }
    for (VROUniform *uniform : _shaderModifierUniforms) {
        uniform->set(nullptr, &geometry, &material);
    }
}
