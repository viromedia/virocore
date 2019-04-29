//
//  VROMaterial.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMaterial.h"
#include "VROAnimationFloat.h"
#include "VROMaterialSubstrate.h"
#include "VRODriver.h"
#include "VROTransaction.h"
#include "VROAllocationTracker.h"
#include "VROSortKey.h"
#include "VROThreadRestricted.h"
#include "VROLog.h"
#include <atomic>
#include <algorithm>

static std::atomic_int sMaterialId;

VROMaterial::VROMaterial() : VROThreadRestricted(VROThreadName::Renderer),
    _materialId(sMaterialId++),
    _shininess(2.0),
    _fresnelExponent(1.0),
    _transparency(1.0),
    _transparencyMode(VROTransparencyMode::AOne),
    _lightingModel(VROLightingModel::Constant),
    _litPerPixel(true),
    _cullMode(VROCullMode::Back),
    _blendMode(VROBlendMode::Alpha),
    _writesToDepthBuffer(true),
    _readsFromDepthBuffer(true),
    _colorWriteMask(VROColorMaskAll),
    _bloomThreshold(-1),
    _postProcessMask(false),
    _receivesShadows(true),
    _castsShadows(true),
    _chromaKeyFilteringEnabled(false),
    _chromaKeyFilteringColor({ 0, 1, 0 }),
    _needsToneMapping(true),
    _renderingOrder(0),
    _substrate(nullptr) {
   
    _diffuse          = new VROMaterialVisual(*this, (int)VROTextureType::None |
                                                     (int)VROTextureType::Texture2D |
                                                     (int)VROTextureType::TextureCube |
                                                     (int)VROTextureType::TextureEGLImage);
    _roughness        = new VROMaterialVisual(*this, (int)VROTextureType::None |
                                                     (int)VROTextureType::Texture2D,
                                              { 0.484529, 0.484529, 0.484529, 1.0 }); // Sensible default for shapes
    _metalness        = new VROMaterialVisual(*this, (int)VROTextureType::None |
                                                     (int)VROTextureType::Texture2D,
                                              { 0, 0, 0, 1.0 });
    _specular         = new VROMaterialVisual(*this, (int)VROTextureType::Texture2D);
    _normal           = new VROMaterialVisual(*this, (int)VROTextureType::Texture2D);
    _reflective       = new VROMaterialVisual(*this, (int)VROTextureType::TextureCube);
    _ambientOcclusion = new VROMaterialVisual(*this, (int)VROTextureType::None |
                                                     (int)VROTextureType::Texture2D);
        
    // TODO These are not yet implemented
    _emission         = new VROMaterialVisual(*this, (int)VROTextureType::Texture2D);
    _multiply         = new VROMaterialVisual(*this, (int)VROTextureType::Texture2D);
    _selfIllumination = new VROMaterialVisual(*this, (int)VROTextureType::Texture2D);
        
    ALLOCATION_TRACKER_ADD(Materials, 1);
}

VROMaterial::VROMaterial(std::shared_ptr<VROMaterial> material) : VROThreadRestricted(VROThreadName::Renderer),
 _materialId(sMaterialId++),
 _name(material->_name),
 _shininess(material->_shininess),
 _fresnelExponent(material->_fresnelExponent),
 _transparency(material->_transparency),
 _transparencyMode(material->_transparencyMode),
 _lightingModel(material->_lightingModel),
 _litPerPixel(material->_litPerPixel),
 _cullMode(material->_cullMode),
 _blendMode(material->_blendMode),
 _writesToDepthBuffer(material->_writesToDepthBuffer),
 _readsFromDepthBuffer(material->_readsFromDepthBuffer),
 _colorWriteMask(material->_colorWriteMask),
 _bloomThreshold(material->_bloomThreshold),
 _postProcessMask(material->_postProcessMask),
 _receivesShadows(material->_receivesShadows),
 _castsShadows(material->_castsShadows),
 _chromaKeyFilteringEnabled(material->_chromaKeyFilteringEnabled),
 _chromaKeyFilteringColor(material->_chromaKeyFilteringColor),
 _needsToneMapping(material->needsToneMapping()),
 _renderingOrder(material->_renderingOrder),
 _substrate(nullptr) {
 
     _diffuse = new VROMaterialVisual(*this, *material->_diffuse);
     _roughness = new VROMaterialVisual(*this, *material->_roughness);
     _metalness = new VROMaterialVisual(*this, *material->_metalness);
     _specular = new VROMaterialVisual(*this, *material->_specular);
     _normal = new VROMaterialVisual(*this, *material->_normal);
     _reflective = new VROMaterialVisual(*this, *material->_reflective);
     _emission = new VROMaterialVisual(*this, *material->_emission);
     _multiply = new VROMaterialVisual(*this, *material->_multiply);
     _ambientOcclusion = new VROMaterialVisual(*this, *material->_ambientOcclusion);
     _selfIllumination = new VROMaterialVisual(*this, *material->_selfIllumination);
     
     ALLOCATION_TRACKER_ADD(Materials, 1);
}

VROMaterial::~VROMaterial() {
    delete (_diffuse);
    delete (_roughness);
    delete (_metalness);
    delete (_specular);
    delete (_normal);
    delete (_reflective);
    delete (_emission);
    delete (_multiply);
    delete (_ambientOcclusion);
    delete (_selfIllumination);
    delete (_substrate);
    
    ALLOCATION_TRACKER_SUB(Materials, 1);
}

void VROMaterial::deleteGL() {
    _diffuse->deleteGL();
    _roughness->deleteGL();
    _metalness->deleteGL();
    _specular->deleteGL();
    _normal->deleteGL();
    _reflective->deleteGL();
    _emission->deleteGL();
    _multiply->deleteGL();
    _ambientOcclusion->deleteGL();
    _selfIllumination->deleteGL();
}

void VROMaterial::copyFrom(std::shared_ptr<VROMaterial> material) {
    _name = material->_name;
    _shininess = material->_shininess;
    _fresnelExponent = material->_fresnelExponent;
    _transparency = material->_transparency;
    _transparencyMode = material->_transparencyMode;
    _lightingModel = material->_lightingModel;
    _litPerPixel = material->_litPerPixel;
    _cullMode = material->_cullMode;
    _blendMode = material->_blendMode;
    _writesToDepthBuffer = material->_writesToDepthBuffer;
    _readsFromDepthBuffer = material->_readsFromDepthBuffer;
    _colorWriteMask = material->_colorWriteMask;
    _bloomThreshold = material->_bloomThreshold;
    _postProcessMask = material->_postProcessMask;
    _receivesShadows = material->_receivesShadows;
    _castsShadows = material->_castsShadows;
    _chromaKeyFilteringEnabled = material->_chromaKeyFilteringEnabled;
    _chromaKeyFilteringColor = material->_chromaKeyFilteringColor;
    _needsToneMapping = material->_needsToneMapping;
    _renderingOrder = material->_renderingOrder;
    
    _substrate = nullptr;
    
    _diffuse->copyFrom(*material->_diffuse);
    _roughness->copyFrom(*material->_roughness);
    _metalness->copyFrom(*material->_metalness);
    _specular->copyFrom(*material->_specular);
    _normal->copyFrom(*material->_normal);
    _reflective->copyFrom(*material->_reflective);
    _emission->copyFrom(*material->_emission);
    _multiply->copyFrom(*material->_multiply);
    _ambientOcclusion->copyFrom(*material->_ambientOcclusion);
    _selfIllumination->copyFrom(*material->_selfIllumination);
}

void VROMaterial::prewarm(std::shared_ptr<VRODriver> driver) {
    getSubstrate(driver);
}

int VROMaterial::hydrateAsync(std::function<void()> callback,
                              std::shared_ptr<VRODriver> &driver) {
    int count = 0;
    VROMaterialVisual *visuals[10] = { _diffuse, _roughness, _metalness, _specular, _normal, _reflective,
                                       _emission, _multiply, _ambientOcclusion, _selfIllumination };
    for (int i = 0; i < 10; i++) {
        VROMaterialVisual *visual = visuals[i];
        if (visual->getTextureType() != VROTextureType::None) {
            if (!visual->getTexture()->isHydrated()) {
                visual->getTexture()->hydrateAsync(callback, driver);
                ++count;
            }
        }
    }
    return count;
}

bool VROMaterial::isHydrated() {
    if (_substrate == nullptr) {
        return false;
    }
    VROMaterialVisual *visuals[10] = { _diffuse, _roughness, _metalness, _specular, _normal, _reflective,
                                       _emission, _multiply, _ambientOcclusion, _selfIllumination };
    for (int i = 0; i < 10; i++) {
        VROMaterialVisual *visual = visuals[i];
        if (visual->getTextureType() != VROTextureType::None) {
            if (!visual->getTexture()->isHydrated()) {
                return false;
            }
        }
    }
    return true;
}

void VROMaterial::setTransparency(float transparency) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float v) {
        ((VROMaterial *)animatable)->_transparency = v;
    }, _transparency, transparency));
}

void VROMaterial::setShininess(float shininess) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float v) {
        ((VROMaterial *)animatable)->_shininess = v;
    }, _shininess, shininess));
}

void VROMaterial::setFresnelExponent(float fresnelExponent) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float v) {
        ((VROMaterial *)animatable)->_fresnelExponent = v;
    }, _fresnelExponent, fresnelExponent));
}

void VROMaterial::fadeSnapshot() {
    if (VROThreadRestricted::isThread(VROThreadName::Renderer)) {
        std::shared_ptr<VROTransaction> transaction = VROTransaction::get();
        if (transaction && !transaction->isDegenerate()) {
            std::shared_ptr<VROMaterial> shared = std::static_pointer_cast<VROMaterial>(shared_from_this());
            std::shared_ptr<VROMaterial> outgoing = std::make_shared<VROMaterial>(shared);
            _outgoing = outgoing;

            // Fade the incoming material (this material) in, up to its previous transparency
            float previousTransparency = _transparency;
            _transparency = 0.0;
            animate(std::make_shared<VROAnimationFloat>(
                    [](VROAnimatable *const animatable, float v) {
                        ((VROMaterial *) animatable)->_transparency = v;
                    },
                    0.0, previousTransparency,
                    [outgoing](VROAnimatable *const animatable) {
                        VROMaterial *material = ((VROMaterial *) animatable);
                        // Ensure we're not removing a more recent animation
                        if (outgoing == material->_outgoing) {
                            material->removeOutgoingMaterial();
                        }
                    }
            ));

            // Fade the outgoing material out as well; it looks better to cross-fade between materials,
            // so there won't be a 'pop' effect when we suddenly remove the outgoing material
            // (when the incoming material is opaque this is not a problem because it completely
            // blocks the outgoing material as its transparency reaches 1.0).

            // VA: Before we had a conditional if(previousTransparency < 1.0), now we fade out regardless
            //     since textures can contain an alpha texture.
            // RA: the problem with removing the previousTransparency conditional is that when we crossfade two
            //     opaque materials, we end up with opacity < 1.0 during the crossfade. e.g., when incoming opacity
            //     is 0.25 and outgoing is 0.75, the total opacity is < 1.0.

            // TODO Figure out how to cross-fade in a way that works for both opaque and transparent materials
            _outgoing->_transparency = previousTransparency;
            _outgoing->animate(std::make_shared<VROAnimationFloat>(
                    [](VROAnimatable *const animatable, float v) {
                        ((VROMaterial *) animatable)->_transparency = v;
                    }, previousTransparency, 0.0));
        }
    }
}

void VROMaterial::addShaderModifier(std::shared_ptr<VROShaderModifier> modifier) {
    _shaderModifiers.push_back(modifier);
    updateSubstrate();
}

void VROMaterial::removeShaderModifier(std::shared_ptr<VROShaderModifier> modifier) {
    _shaderModifiers.erase(std::remove_if(_shaderModifiers.begin(), _shaderModifiers.end(),
                                 [modifier](std::shared_ptr<VROShaderModifier> candidate) {
                                     return candidate == modifier;
                                 }), _shaderModifiers.end());
    updateSubstrate();
}

bool VROMaterial::hasShaderModifier(std::shared_ptr<VROShaderModifier> modifier) {
    for (std::shared_ptr<VROShaderModifier> &candidate : _shaderModifiers) {
        if (modifier == candidate) {
            return true;
        }
    }
    return false;
}

void VROMaterial::removeOutgoingMaterial() {
    _outgoing.reset();
}

void VROMaterial::updateSubstrateTextures() {
    if (_substrate) {
        _substrate->updateTextures();
    }
}

void VROMaterial::updateSubstrate() {
    passert_thread(__func__);
    if (_substrate != nullptr) {
        delete (_substrate);
    }
    _substrate = nullptr;
}

VROMaterialSubstrate *const VROMaterial::getSubstrate(std::shared_ptr<VRODriver> &driver) {
    if (!_substrate) {
        _substrate = driver->newMaterialSubstrate(*this);
    }
    return _substrate;
}

void VROMaterial::updateSortKey(VROSortKey &key, const std::vector<std::shared_ptr<VROLight>> &lights,
                                const VRORenderContext &context,
                                std::shared_ptr<VRODriver> &driver) {
    key.material = _materialId;
    getSubstrate(driver)->updateSortKey(key, lights, context, driver);
}

void VROMaterial::bindProperties(std::shared_ptr<VRODriver> &driver) {
    driver->setCullMode(_cullMode);
    driver->setDepthReadingEnabled(_readsFromDepthBuffer);
    driver->setDepthWritingEnabled(_writesToDepthBuffer);
    driver->setMaterialColorWritingMask(_colorWriteMask);
    driver->setBlendingMode(_blendMode);
    getSubstrate(driver)->bindProperties(driver);
}

bool VROMaterial::bindShader(int lightsHash,
                             const std::vector<std::shared_ptr<VROLight>> &lights,
                             const VRORenderContext &context,
                             std::shared_ptr<VRODriver> &driver) {
    return getSubstrate(driver)->bindShader(lightsHash, lights, context, driver);
}

bool VROMaterial::hasDiffuseAlpha() const {
    if (_diffuse->getTextureType() == VROTextureType::None) {
        return _diffuse->getColor().w < (1.0 - kEpsilon);
    }
    else {
        return _diffuse->getColor().w < (1.0 - kEpsilon) ||
               _diffuse->getTexture()->hasAlpha();
    }
}

void VROMaterial::setChromaKeyFilteringEnabled(bool enabled) {
    _chromaKeyFilteringEnabled = enabled;
    updateSubstrate();
}

void VROMaterial::setChromaKeyFilteringColor(VROVector3f color) {
    _chromaKeyFilteringColor = color;
    updateSubstrate();
}

void VROMaterial::setNeedsToneMapping(bool needsToneMapping) {
    _needsToneMapping = needsToneMapping;
}
