//
//  VROMaterialVisual.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMaterialVisual.h"
#include "VROAnimationFloat.h"

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
    NSLog(@"Intensity: %f", intensity);
    _heartbeat->animate(std::make_shared<VROAnimationFloat>([this](float value) {
                                                                _intensity = value;
                                                            }, _intensity, intensity));
}