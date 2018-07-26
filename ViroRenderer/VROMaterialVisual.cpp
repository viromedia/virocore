//
//  VROMaterialVisual.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMaterialVisual.h"
#include "VROAnimationFloat.h"
#include "VROMaterial.h"
#include "VROLog.h"

VROMaterialVisual::VROMaterialVisual(VROMaterial &material, const VROMaterialVisual &visual) :
 _material(material),
 _permissibleContentsMask(visual._permissibleContentsMask),
 _contentsColor(visual._contentsColor),
 _contentsTexture(visual._contentsTexture),
 _intensity(visual._intensity),
 _contentsTransform(visual._contentsTransform)
{}

void VROMaterialVisual::deleteGL() {
    if (_contentsTexture) {
        _contentsTexture->deleteGL();
    }
}

void VROMaterialVisual::copyFrom(const VROMaterialVisual &visual) {
    _permissibleContentsMask = visual._permissibleContentsMask;
    _contentsColor = visual._contentsColor;
    _contentsTexture = visual._contentsTexture;
    _intensity = visual._intensity;
    _contentsTransform = visual._contentsTransform;
}

void VROMaterialVisual::clear() {
    _material.fadeSnapshot();
    
    _contentsColor = { 1.0, 1.0, 1.0, 1.0 };
    _contentsTexture.reset();
    
    _material.updateSubstrate();
}

void VROMaterialVisual::setColor(VROVector4f color) {
    if ((_permissibleContentsMask & (int) VROTextureType::None) == 0) {
        pabort("Material visual does not support fixed contents");
        return;
    }
    _material.fadeSnapshot();
    _contentsColor = color;
    _material.updateSubstrate();
}

void VROMaterialVisual::setTexture(std::shared_ptr<VROTexture> texture) {
    if (texture && (_permissibleContentsMask & (int) texture->getType()) == 0) {
        pabort("Material visual does not support texture of type %d", texture->getType());
        return;
    }

    _material.fadeSnapshot();
    _contentsTexture = texture;
    _material.updateSubstrate();
}

bool VROMaterialVisual::swapTexture(std::shared_ptr<VROTexture> texture) {
    // If this is the first time a texture is assigned, we have to replace the substrate
    if (!_contentsTexture) {
        setTexture(texture);
        return true;
    }
    // Otherwise we can hot-swap
    else {
        _contentsTexture = texture;
        _material.updateSubstrateTextures();
        return false;
    }
}

void VROMaterialVisual::setIntensity(float intensity) {
    _material.fadeSnapshot();
    _intensity = intensity;
    _material.updateSubstrate();
}
