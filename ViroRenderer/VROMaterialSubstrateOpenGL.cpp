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
#include "VROMaterialShaderBinding.h"
#include "VROTextureReference.h"
#include <sstream>

#pragma mark - Loading Materials

VROMaterialSubstrateOpenGL::VROMaterialSubstrateOpenGL(VROMaterial &material, std::shared_ptr<VRODriverOpenGL> &driver) :
    _material(material),
    _activeBinding(nullptr) {

    _materialShaderCapabilities = VROShaderCapabilities::deriveMaterialCapabilitiesKey(material);
    ALLOCATION_TRACKER_ADD(MaterialSubstrates, 1);
}
    
VROMaterialSubstrateOpenGL::~VROMaterialSubstrateOpenGL() {
    ALLOCATION_TRACKER_SUB(MaterialSubstrates, 1);
}

#pragma mark - Binding Materials

void VROMaterialSubstrateOpenGL::bindProperties(std::shared_ptr<VRODriver> &driver) {
    passert(_activeBinding != nullptr);
    _activeBinding->bindMaterialUniforms(_material, driver);
}

void VROMaterialSubstrateOpenGL::bindGeometry(float opacity, const VROGeometry &geometry){
    passert(_activeBinding != nullptr);
    _activeBinding->bindGeometryUniforms(opacity, geometry, _material);
}

bool VROMaterialSubstrateOpenGL::bindShader(int lightsHash,
                                            const std::vector<std::shared_ptr<VROLight>> &lights,
                                            const VRORenderContext &context,
                                            std::shared_ptr<VRODriver> &driver) {
    
    _activeBinding = getShaderBindingForLights(lights, context, driver);
    
    std::shared_ptr<VROShaderProgram> &shader = _activeBinding->getProgram();
    if (!shader->isHydrated()) {
        if (!shader->hydrate()) {
            return false;
        }
    }
    driver->bindShader(shader);
    
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
    _lightingUBO->bind();
    return true;
}

void VROMaterialSubstrateOpenGL::bindView(VROMatrix4f modelMatrix, VROMatrix4f viewMatrix,
                                          VROMatrix4f projectionMatrix, VROMatrix4f normalMatrix,
                                          VROVector3f cameraPosition, VROEyeType eyeType) {
    passert(_activeBinding != nullptr);
    _activeBinding->bindViewUniforms(modelMatrix, viewMatrix, projectionMatrix, normalMatrix,
                                     cameraPosition, eyeType);
}

const std::vector<VROTextureReference> &VROMaterialSubstrateOpenGL::getTextures() const {
    passert (_activeBinding != nullptr);
    return _activeBinding->getTextures();
}

#pragma mark - Updating Sort Key and Textures

void VROMaterialSubstrateOpenGL::updateTextures() {
    for (auto &kv : _shaderBindings) {
        kv.second->loadTextures();
    }
}

void VROMaterialSubstrateOpenGL::updateSortKey(VROSortKey &key, const std::vector<std::shared_ptr<VROLight>> &lights,
                                               const VRORenderContext &context,
                                               std::shared_ptr<VRODriver> driver) {
    VROMaterialShaderBinding *binding = getShaderBindingForLights(lights, context, driver);
    passert (binding != nullptr);

    key.materialRenderingOrder = _material.getRenderingOrder();
    key.shader = binding->getProgram()->getShaderId();
    key.textures = hashTextures(binding->getTextures());
}

VROMaterialShaderBinding *VROMaterialSubstrateOpenGL::getShaderBindingForLights(const std::vector<std::shared_ptr<VROLight>> &lights,
                                                                                const VRORenderContext &context,
                                                                                std::shared_ptr<VRODriver> driver) {
    std::shared_ptr<VRODriverOpenGL> driverGL = std::dynamic_pointer_cast<VRODriverOpenGL>(driver);
    VROLightingShaderCapabilities capabilities = VROShaderCapabilities::deriveLightingCapabilitiesKey(lights, context);
    
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

uint32_t VROMaterialSubstrateOpenGL::hashTextures(const std::vector<VROTextureReference> &textures) const {
    uint32_t h = 0;
    for (const VROTextureReference &texture : textures) {
        if (!texture.isGlobal()) {
            h = 31 * h + texture.getLocalTexture()->getTextureId();
        }
    }
    return h;
}
