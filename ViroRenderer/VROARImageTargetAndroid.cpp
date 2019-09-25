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
    VROARImageTarget(orientation, physicalWidth),
    _id(id) {
    _image = std::make_shared<VROImageAndroid>(bitmapImage);
}

VROARImageTargetAndroid::VROARImageTargetAndroid(std::string id) :
    VROARImageTarget(VROImageOrientation::Up, 0),
    _id(id) {
}

VROARImageTargetAndroid::~VROARImageTargetAndroid() {

}

void VROARImageTargetAndroid::initWithTrackingImpl(VROImageTrackingImpl impl) {
    _currentImpl = impl;

    if (impl == VROImageTrackingImpl::Viro) {
#if ENABLE_OPENCV
        size_t length;
        cv::Mat temp(_image->getHeight(), _image->getWidth(), CV_8UC4, _image->getData(&length));
        setTargetMat(temp);
#endif
    } else if (impl == VROImageTrackingImpl::ARCore) {
        // no-op
    }
}
