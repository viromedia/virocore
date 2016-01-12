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
 _contentsType(visual._contentsType),
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

void VROMaterialVisual::setContents(VROVector4f contents) {
    if ((_permissibleContentsMask & (int) VROContentsType::Fixed) == 0) {
        pabort("Material visual does not support fixed contents");
        return;
    }
    
    _material.fadeSnapshot();
    
    _contentsColor = contents;
    _contentsType = VROContentsType::Fixed;
    
    _material.updateSubstrate();
}

void VROMaterialVisual::setContents(std::shared_ptr<VROTexture> texture) {
    if ((_permissibleContentsMask & (int) VROContentsType::Texture2D) == 0) {
        pabort("Material visual does not support 2D textures");
        return;
    }
    
    _material.fadeSnapshot();
    
    _contentsTexture = texture;
    _contentsType = VROContentsType::Texture2D;
    
    _material.updateSubstrate();
}

void VROMaterialVisual::setContentsCube(std::shared_ptr<VROTexture> texture) {
    if ((_permissibleContentsMask & (int) VROContentsType::TextureCube) == 0) {
        pabort("Material visual does not support cube textures");
        return;
    }
    
    _material.fadeSnapshot();
    
    _contentsTexture = texture;
    _contentsType = VROContentsType::TextureCube;
    
    _material.updateSubstrate();
}

void VROMaterialVisual::setIntensity(float intensity) {
    // TODO Migrate this to the snapshot system
    
    _heartbeat->animate(std::make_shared<VROAnimationFloat>([this](float value) {
                                                                _intensity = value;
                                                            }, _intensity, intensity));
}