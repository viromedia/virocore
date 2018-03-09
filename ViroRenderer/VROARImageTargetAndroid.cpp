//
//  VROARImageTargetAndroid.cpp
//  ViroKit
//
//  Created by Andy Chu on 3/7/18.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARImageTargetAndroid.h"
#include "VROImageAndroid.h"

VROARImageTargetAndroid::VROARImageTargetAndroid(jobject bitmapImage, VROImageOrientation orientation,
                                                 float physicalWidth, std::string id) :
    _orientation(orientation),
    _physicalWidth(physicalWidth),
    _id(id) {
    _image = std::make_shared<VROImageAndroid>(bitmapImage);
}

VROARImageTargetAndroid::~VROARImageTargetAndroid() {

}

void VROARImageTargetAndroid::initWithTrackingImpl(VROImageTrackingImpl impl) {
    _currentImpl = impl;

    if (impl == VROImageTrackingImpl::Viro) {
        // TODO: create the the target
    }
}
