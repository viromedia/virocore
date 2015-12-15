//
//  VROGeometry.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROGeometry.h"
#include "VROGeometrySubstrate.h"
#include "VRORenderParameters.h"

VROGeometry::~VROGeometry() {
    delete (_substrate);
}

void VROGeometry::render(const VRORenderContext &context,
                         VRORenderParameters &params) {
    
    if (!_substrate) {
        _substrate = context.newGeometrySubstrate(*this);
    }
    _substrate->render(context, params);
}