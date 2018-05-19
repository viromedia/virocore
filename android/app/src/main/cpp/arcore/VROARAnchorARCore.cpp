//
//  VROARAnchorARCore.cpp
//  ViroKit
//
//  Created by Raj Advani on 9/24/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARAnchorARCore.h"
#include "VROMatrix4f.h"

VROARAnchorARCore::VROARAnchorARCore(std::string key,
                                     std::shared_ptr<arcore::Anchor> anchor,
                                     std::shared_ptr<VROARAnchor> trackable) :
    _anchor(anchor),
    _trackable(trackable) {

    setId(key);
}

VROARAnchorARCore::~VROARAnchorARCore() {
}


std::shared_ptr<arcore::Anchor> VROARAnchorARCore::getAnchorInternal() {
    return _anchor;
}

std::shared_ptr<VROARAnchor> VROARAnchorARCore::getTrackable() {
    return _trackable;
}
