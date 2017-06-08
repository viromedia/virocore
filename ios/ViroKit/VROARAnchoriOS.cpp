//
//  VROARAnchoriOS.cpp
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "Availability.h"
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000
#include "VROARAnchoriOS.h"
#include "VROConvert.h"

VROARAnchoriOS::VROARAnchoriOS(ARAnchor *anchor) :
    _anchor(anchor) {
        
}

VROARAnchoriOS::~VROARAnchoriOS() {
    
}

VROMatrix4f VROARAnchoriOS::getTransform() const {
    return VROConvert::toMatrix4f(_anchor.transform);
}

#endif
