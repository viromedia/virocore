//
//  VROBone.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROBone.h"
#include "VROAnimationMatrix4f.h"

void VROBone::setTransform(VROMatrix4f transform) {
    animate(std::make_shared<VROAnimationMatrix4f>([](VROAnimatable *const animatable, VROMatrix4f p) {
        ((VROBone *)animatable)->_transform = p;
    }, _transform, transform));
}
