//
//  VROMaterialSubstrateOpenGL.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROMaterialSubstrateOpenGL.h"
#include "VRODriverOpenGL.h"
#include "VROMaterial.h"
#include "VROAllocationTracker.h"

VROMaterialSubstrateOpenGL::VROMaterialSubstrateOpenGL(const VROMaterial &material, const VRODriverOpenGL &driver) :
    _material(material),
    _lightingModel(material.getLightingModel()) {

    ALLOCATION_TRACKER_ADD(MaterialSubstrates, 1);
}
    
VROMaterialSubstrateOpenGL::~VROMaterialSubstrateOpenGL() {
    ALLOCATION_TRACKER_SUB(MaterialSubstrates, 1);
}