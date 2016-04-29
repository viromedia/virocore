//
//  VRODriverCardboard.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VRODriverCardboard.h"
#import "VROViewport.h"
#import "VROFieldOfView.h"
#import "VRODriverContextMetal.h"

VRODriverCardboard::VRODriverCardboard(std::shared_ptr<VRORenderer> renderer) :
    _renderer(renderer) {
        
   
}

VRODriverCardboard::~VRODriverCardboard() {
    
}

void VRODriverCardboard::onOrientationChange(UIInterfaceOrientation orientation) {
    
}

VROViewport VRODriverCardboard::getViewport(VROEyeType eye) {
    return {};
}

VROFieldOfView VRODriverCardboard::getFOV(VROEyeType eye) {
    return {};
}
