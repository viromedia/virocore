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

static std::atomic_int sMaterialId;

VROMaterial::VROMaterial() :
    _materialId(sMaterialId++),
    _shininess(2.0),
    _fresnelExponent(1.0),
    _transparency(1.0),
    _transparencyMode(VROTransparencyMode::AOne),
    _lightingModel(VROLightingModel::Blinn),
    _litPerPixel(true),
    _cullMode(VROCullMode::None),
    _blendMode(VROBlendMode::Alpha),
    _writesToDepthBuffer(false),
    _readsFromDepthBuffer(false),
    _substrate(nullptr) {
    
    _diffuse          = new VROMaterialVisual(*this, (int)VROContentsType::Fixed | (int)VROContentsType::Texture2D | (int)VROContentsType::TextureCube);
    _specular         = new VROMaterialVisual(*this, (int)VROContentsType::Texture2D);
    _normal           = new VROMaterialVisual(*this, (int)VROContentsType::Texture2D);
    _reflective       = new VROMaterialVisual(*this, (int)VROContentsType::TextureCube);
        
    // TODO These are not yet implemented
    _emission         = new VROMaterialVisual(*this, (int)VROContentsType::Texture2D);
    _transparent      = new VROMaterialVisual(*this, (int)VROContentsType::Texture2D);
    _multiply         = new VROMaterialVisual(*this, (int)VROContentsType::Texture2D);
    _ambientOcclusion = new VROMaterialVisual(*this, (int)VROContentsType::Texture2D);
    _selfIllumination = new VROMaterialVisual(*this, (int)VROContentsType::Texture2D);
        
    ALLOCATION_TRACKER_ADD(Materials, 1);
}

VROMaterial::VROMaterial(std::shared_ptr<VROMaterial> material) :
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
 _substrate(nullptr) {
 
     _diffuse = new VROMaterialVisual(*material->_diffuse);
     _specular = new VROMaterialVisual(*material->_specular);
     _normal = new VROMaterialVisual(*material->_normal);
     _reflective = new VROMaterialVisual(*material->_reflective);
     _emission = new VROMaterialVisual(*material->_emission);
     _transparent = new VROMaterialVisual(*material->_transparent);
     _multiply = new VROMaterialVisual(*material->_multiply);
     _ambientOcclusion = new VROMaterialVisual(*material->_ambientOcclusion);
     _selfIllumination = new VROMaterialVisual(*material->_selfIllumination);
     
     ALLOCATION_TRACKER_ADD(Materials, 1);
}

VROMaterial::~VROMaterial() {
    delete (_diffuse);
    delete (_specular);
    delete (_normal);
    delete (_reflective);
    delete (_emission);
    delete (_transparent);
    delete (_multiply);
    delete (_ambientOcclusion);
    delete (_selfIllumination);
    delete (_substrate);
    
    ALLOCATION_TRACKER_SUB(Materials, 1);
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
    std::shared_ptr<VROTransaction> transaction = VROTransaction::get();
    if (transaction && !transaction->isDegenerate()) {
        std::shared_ptr<VROMaterial> shared = std::static_pointer_cast<VROMaterial>(shared_from_this());
        std::shared_ptr<VROMaterial> outgoing = std::make_shared<VROMaterial>(shared);
        _outgoing = outgoing;
        
        _transparency = 0.0;
        animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float v) {
                                                        ((VROMaterial *)animatable)->_transparency = v;
                                                    },
                                                    0.0, 1.0,
                                                    [outgoing](VROAnimatable *const animatable) {
                                                        VROMaterial *material = ((VROMaterial *)animatable);
                                                        // Ensure we're not removing a more recent animation
                                                        if (outgoing == material->_outgoing) {
                                                            material->removeOutgoingMaterial();
                                                        }
                                                    }
                                                    ));
        
        _outgoing->_transparency = 1.0;
        _outgoing->animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float v) {
            ((VROMaterial *)animatable)->_transparency = v;
        }, 1.0, 0.0));
    }
}

void VROMaterial::removeOutgoingMaterial() {
    _outgoing.reset();
}

void VROMaterial::updateSubstrate() {
    delete (_substrate);
    _substrate = nullptr;
}

void VROMaterial::createSubstrate(const VRODriver &driver) {
    if (!_substrate) {
        _substrate = driver.newMaterialSubstrate(*this);
    }
}

void VROMaterial::updateSortKey(VROSortKey &key) {
    key.material = _materialId;
    
    if (_substrate) {
        _substrate->updateSortKey(key);
    }
    else {
        key.shader = 0;
        key.textures = 0;
    }
}

void VROMaterial::bindShader(const VRODriver &driver) {
    createSubstrate(driver);
    _substrate->bindShader();
}

void VROMaterial::bindLights(const std::vector<std::shared_ptr<VROLight>> &lights,
                             const VRORenderContext &context, const VRODriver &driver) {
    createSubstrate(driver);
    _substrate->bindLights(lights, context, driver);
}
