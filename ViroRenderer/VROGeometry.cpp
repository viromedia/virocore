//
//  VROGeometry.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROGeometry.h"
#include "VROGeometrySubstrate.h"

VROGeometry::~VROGeometry() {
    delete (_substrate);
}

void VROGeometry::render(const VRORenderContext &context,
                         const VROMatrix4f &rotation,
                         const VROMatrix4f &transform,
                         const std::vector<std::shared_ptr<VROLight>> &lights) {
    
    if (!_substrate) {
        _substrate = context.newGeometrySubstrate(*this);
    }
    _substrate->render(context, rotation, transform, lights);
}