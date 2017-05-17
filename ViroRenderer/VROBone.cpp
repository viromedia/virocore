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
    _transform = transform;
}
