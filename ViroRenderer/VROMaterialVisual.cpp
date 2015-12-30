//
//  VROMaterialVisual.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMaterialVisual.h"
#include "VROAnimationFloat.h"

VROMaterialVisual::VROMaterialVisual(const VROMaterialVisual &visual) :
 _material(visual._material),
 _heartbeat(std::make_shared<VROMaterialVisualHeartbeat>()),
 _contentsType(visual._contentsType),
 _contentsColor(visual._contentsColor),
 _contentsTexture(visual._contentsTexture),
 _contentsCube(visual._contentsCube),
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
    _contentsColor = contents;
    _contentsType = VROContentsType::Fixed;
}

void VROMaterialVisual::setContents(std::shared_ptr<VROTexture> texture) {
    _contentsTexture = texture;
    _contentsType = VROContentsType::Texture2D;
}

void VROMaterialVisual::setContents(std::vector<std::shared_ptr<VROTexture>> cubeTextures) {
    _contentsCube = cubeTextures;
    _contentsType = VROContentsType::TextureCube;
}

void VROMaterialVisual::setIntensity(float intensity) {
    _heartbeat->animate(std::make_shared<VROAnimationFloat>([this](float value) {
                                                                _intensity = value;
                                                            }, _intensity, intensity));
}