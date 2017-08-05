//
//  VROPortalFrame.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPortalFrame.h"

VROPortalFrame::VROPortalFrame() :
    _twoSided(false) {
    
}

VROPortalFrame::~VROPortalFrame() {
    
}

VROFace VROPortalFrame::getActiveFace(bool isExit) const {
    if (_twoSided) {
        if (isExit) {
            return VROFace::Back;
        }
        else {
            return VROFace::Front;
        }
    }
    else {
        return VROFace::FrontAndBack;
    }
}

VROFace VROPortalFrame::getInactiveFace(bool isExit) const {
    VROFace active = getActiveFace(isExit);
    switch (active) {
        case VROFace::Front:
            return VROFace::Back;
        case VROFace::Back:
            return VROFace::Front;
        default:
            return VROFace::FrontAndBack;
    }
}
