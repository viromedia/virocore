//
//  VROImageAndroid.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/10/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROImageAndroid.h"
#include "VROPlatformUtil.h"
#include <stdlib.h>

VROImageAndroid::VROImageAndroid(std::string asset, VROTextureInternalFormat internalFormat) {
    jobject jbitmap = VROPlatformLoadBitmapFromAsset(asset, internalFormat);
    _data = (unsigned char *)VROPlatformConvertBitmap(jbitmap, &_dataLength, &_width, &_height);

    if (internalFormat == VROTextureInternalFormat::RGB565) {
        _format = VROTextureFormat::RGB565;
    }
    else {
        _format = VROTextureFormat::RGBA8;
    }
}

VROImageAndroid::VROImageAndroid(jobject jbitmap, VROTextureInternalFormat internalFormat) {
    _data = (unsigned char *)VROPlatformConvertBitmap(jbitmap, &_dataLength, &_width, &_height);
    if (internalFormat == VROTextureInternalFormat::RGB565) {
        _format = VROTextureFormat::RGB565;
    }
    else {
        _format = VROTextureFormat::RGBA8;
    }
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

unsigned char *VROImageAndroid::getData(size_t *length) {
    *length = _dataLength;
    return _data;
}