//
//  VROMaterial.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMaterial.h"
#include "VROAnimationFloat.h"

void VROMaterial::setShininess(float shininess) {
    animate(std::make_shared<VROAnimationFloat>([this](float v) {
        _shininess = v;
    }, _shininess, shininess));
}