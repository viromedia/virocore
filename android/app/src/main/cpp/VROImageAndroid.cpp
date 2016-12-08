//
//  VROImageAndroid.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/10/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROImageAndroid.h"
#include "VROPlatformUtil.h"


VROImageAndroid::VROImageAndroid(std::string resource) {
    _data = (unsigned char *)VROPlatformLoadImageAssetRGBA8888(resource, &_dataLength, &_width, &_height);
}

VROImageAndroid::VROImageAndroid(jobject jbitmap) {
    _data = (unsigned char *)VROPlatformConvertBitmap(jbitmap, &_dataLength, &_width, &_height);
}

VROImageAndroid::~VROImageAndroid() {
    free(_data);
}

int VROImageAndroid::getWidth() const {
    return _width;
}

int VROImageAndroid::getHeight() const {
    return _height;
}

unsigned char *VROImageAndroid::extractRGBA8888(size_t *length) {
    *length = _dataLength;
    return _data;
}