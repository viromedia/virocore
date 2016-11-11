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

VROMaterialVisual::VROMaterialVisual(const VROMaterialVisual &visual) :
 _material(visual._material),
 _permissibleContentsMask(visual._permissibleContentsMask),
 _heartbeat(std::make_shared<VROMaterialVisualHeartbeat>()),
 _contentsColor(visual._contentsColor),
 _contentsTexture(visual._contentsTexture),
 _intensity(visual._intensity),
 _contentsTransform(visual._contentsTransform),
 _wrapS(visual._wrapS),
 _wrapT(visual._wrapT),
 _minificationFilter(visual._minificationFilter),
 _magnificationFilter(visual._magnificationFilter),
 _mipFilter(visual._mipFilter),
 _borderColor(visual._borderColor)
{}

void VROMaterialVisual::clear() {
    _material.fadeSnapshot();
    
    _contentsColor = { 1.0, 1.0, 1.0, 1.0 };
    _contentsTexture.reset();
    
    _material.updateSubstrate();
}

void VROMaterialVisual::setContents(VROVector4f contents) {
    if ((_permissibleContentsMask & (int) VROTextureType::None) == 0) {
        pabort("Material visual does not support fixed contents");
        return;
    }
    
    _material.fadeSnapshot();
    
    _contentsColor = contents;
    _contentsTexture.reset();
    
    _material.updateSubstrate();
}

void VROMaterialVisual::setContents(std::shared_ptr<VROTexture> texture) {
    if ((_permissibleContentsMask & (int) texture->getType()) == 0) {
        pabort("Material visual does not support texture of type %d", texture->getType());
        return;
    }
    
    _material.fadeSnapshot();
    
    _contentsTexture = texture;
    _material.updateSubstrate();
}

void VROMaterialVisual::setIntensity(float intensity) {
    // TODO Migrate this to the snapshot system
    
    _heartbeat->animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float value) {
                                                                ((VROMaterialVisual *)animatable)->_intensity = value;
                                                            }, _intensity, intensity));
}
