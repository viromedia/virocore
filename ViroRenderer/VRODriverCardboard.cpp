//
//  VRODriverCardboard.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VRODriverCardboard.h"
#include "VROViewport.h"
#include "VROFieldOfView.h"

VRODriverCardboard::VRODriverCardboard() {
    
}

VRODriverCardboard::~VRODriverCardboard() {
    
}

UIView *VRODriverCardboard::getRenderingView() {
    return nullptr;
}

void VRODriverCardboard::onOrientationChange(UIInterfaceOrientation orientation) {
    
}

VROViewport VRODriverCardboard::getViewport(VROEyeType eye) {
    return {};
}

VROFieldOfView VRODriverCardboard::getFOV(VROEyeType eye) {
    return {};
}
