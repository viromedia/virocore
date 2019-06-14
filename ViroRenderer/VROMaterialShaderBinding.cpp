//
//  VROMaterialShaderBinding.cpp
//  ViroKit
//
//  Created by Raj Advani on 1/24/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROMaterialShaderBinding.h"
#include "VROShaderProgram.h"
#include "VROShaderModifier.h"
#include "VROUniform.h"
#include "VROVector3f.h"
#include "VROMatrix4f.h"
#include "VROMaterial.h"
#include "VROEye.h"
#include "VRODriver.h"
#include "VROTextureReference.h"
#include "VROMath.h"

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
    _roughnessUniform(nullptr),
    _roughnessIntensityUniform(nullptr),
    _metalnessUniform(nullptr),
    _metalnessIntensityUniform(nullptr),
    _aoUniform(nullptr),
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
    _roughnessUniform = program->getUniform("material_roughness");
    _roughnessIntensityUniform = program->getUniform("material_roughness_intensity");
    _metalnessUniform = program->getUniform("material_metalness");
    _metalnessIntensityUniform = program->getUniform("material_metalness_intensity");
    _aoUniform = program->getUniform("material_ao");
    
    _normalMatrixUniform = program->getUniform("normal_matrix");
    _modelMatrixUniform = program->getUniform("model_matrix");
    _projectionMatrixUniform = program->getUniform("projection_matrix");
    _viewMatrixUniform = program->getUniform("view_matrix");
    _cameraPositionUniform = program->getUniform("camera_position");
    _eyeTypeUniform = program->getUniform("eye_type");
    
    for (const std::shared_ptr<VROShaderModifier> &modifier : program->getModifiers()) {
        std::vector<std::string> uniformNames = modifier->getUniforms();
        
        for (std::string &uniformName : uniformNames) {
            VROUniformBinder *binder = modifier->getUniformBinder(uniformName);
            VROUniform *uniform = _program->getUniform(binder->getName());
            passert_msg (binder != nullptr, "Failed to find binder for uniform '%s' in program!",
                         uniformName.c_str());
            passert_msg (uniform != nullptr, "Failed to find uniform for modifier uniform '%s' in program!",
                         uniformName.c_str());
            _modifierUniformBinders.push_back({ binder, uniform });
        }
    }
}

void VROMaterialShaderBinding::loadTextures() {
    _textures.clear();
    
    const std::vector<std::string> &samplers = _program->getSamplers();
    for (const std::string &sampler : samplers) {
        if (sampler == "diffuse_texture" || sampler == "diffuse_texture_y") {
            _textures.emplace_back(_material.getDiffuse().getTexture());
        }
        else if (sampler == "specular_texture") {
            _textures.emplace_back(_material.getSpecular().getTexture());
        }
        else if (sampler == "normal_texture") {
            _textures.emplace_back(_material.getNormal().getTexture());
        }
        else if (sampler == "reflect_texture") {
            _textures.emplace_back(_material.getReflective().getTexture());
        }
        else if (sampler == "roughness_map") {
            _textures.emplace_back(_material.getRoughness().getTexture());
        }
        else if (sampler == "metalness_map") {
            _textures.emplace_back(_material.getMetalness().getTexture());
        }
        else if (sampler == "ao_map") {
            _textures.emplace_back(_material.getAmbientOcclusion().getTexture());
        }
        else if (sampler == "shadow_map") {
            _textures.emplace_back(VROGlobalTextureType::ShadowMap);
        }
        else if (sampler == "irradiance_map") {
            _textures.emplace_back(VROGlobalTextureType::IrradianceMap);
        }
        else if (sampler == "prefiltered_map") {
            _textures.emplace_back(VROGlobalTextureType::PrefilteredMap);
        }
        else if (sampler == "brdf_map") {
            _textures.emplace_back(VROGlobalTextureType::BrdfMap);
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
    if (_eyeTypeUniform != nullptr) {
        if (eyeType == VROEyeType::Left || eyeType == VROEyeType::Monocular) {
            _eyeTypeUniform->setFloat(0);
        }
        else {
            _eyeTypeUniform->setFloat(1.0);
        }
    }
}

void VROMaterialShaderBinding::bindMaterialUniforms(const VROMaterial &material,
                                                    std::shared_ptr<VRODriver> &driver) {
    if (_diffuseSurfaceColorUniform != nullptr) {
        VROVector4f color = material.getDiffuse().getColor();
        if (driver->isLinearRenderingEnabled()) {
            color = VROMathConvertSRGBToLinearColor(color);
        }
        _diffuseSurfaceColorUniform->setVec4(color);
    }
    if (_diffuseIntensityUniform != nullptr) {
        _diffuseIntensityUniform->setFloat(material.getDiffuse().getIntensity());
    }
    if (_shininessUniform != nullptr) {
        _shininessUniform->setFloat(material.getShininess());
    }
    if (_roughnessUniform != nullptr) {
        _roughnessUniform->setFloat(material.getRoughness().getColor().x);
    }
    if (_roughnessIntensityUniform != nullptr) {
        _roughnessIntensityUniform->setFloat(material.getRoughness().getIntensity());
    }
    if (_metalnessUniform != nullptr) {
        _metalnessUniform->setFloat(material.getMetalness().getColor().x);
    }
    if (_metalnessIntensityUniform != nullptr) {
        _metalnessIntensityUniform->setFloat(material.getMetalness().getIntensity());
    }
    if (_aoUniform != nullptr) {
        _aoUniform->setFloat(material.getAmbientOcclusion().getColor().x);
    }
}

void VROMaterialShaderBinding::bindGeometryUniforms(float opacity, const VROGeometry &geometry, const VROMaterial &material) {
    if (_alphaUniform != nullptr) {
        _alphaUniform->setFloat(material.getTransparency() * opacity);
    }
    for (auto binder_uniform : _modifierUniformBinders) {
        binder_uniform.first->setForMaterial(binder_uniform.second, &geometry, &material);
    }
}
