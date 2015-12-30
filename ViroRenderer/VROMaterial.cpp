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
#include "VRORenderContext.h"
#include "VROTransaction.h"

VROMaterial::VROMaterial() :
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
    _outgoingAlpha(0.0),
    _substrate(nullptr) {
    
    _diffuse = new VROMaterialVisual(*this);
    _specular = new VROMaterialVisual(*this);
    _normal = new VROMaterialVisual(*this);
    _reflective = new VROMaterialVisual(*this);
    _emission = new VROMaterialVisual(*this);
    _transparent = new VROMaterialVisual(*this);
    _multiply = new VROMaterialVisual(*this);
    _ambientOcclusion = new VROMaterialVisual(*this);
    _selfIllumination = new VROMaterialVisual(*this);
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
}

VROMaterial::VROMaterial(std::shared_ptr<VROMaterial> material) :
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
}

void VROMaterial::setShininess(float shininess) {
    animate(std::make_shared<VROAnimationFloat>([this](float v) {
        _shininess = v;
    }, _shininess, shininess));
}

void VROMaterial::setFresnelExponent(float fresnelExponent) {
    animate(std::make_shared<VROAnimationFloat>([this](float v) {
        _fresnelExponent = v;
    }, _fresnelExponent, fresnelExponent));
}

void VROMaterial::snapshotOutgoing() {
    std::shared_ptr<VROTransaction> transaction = VROTransaction::get();
    if (transaction) {        
        if (!_outgoing) {
            _outgoing = std::make_shared<VROMaterial>(std::static_pointer_cast<VROMaterial>(shared_from_this()));
            _outgoingAlpha = 1.0;
        }
    }
}

VROMaterialSubstrate *const VROMaterial::getSubstrate(const VRORenderContext &context) {
    if (!_substrate) {
        _substrate = context.newMaterialSubstrate(*this);
    }
    
    return _substrate;
}