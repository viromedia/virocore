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
    bool hasAlpha;
    _data = (unsigned char *)VROPlatformConvertBitmap(jbitmap, &_dataLength, &_width, &_height, &hasAlpha);

    if (internalFormat == VROTextureInternalFormat::RGB565) {
        _format = VROTextureFormat::RGB565;
        _internalFormat = VROTextureInternalFormat::RGB565;
    } else {
        // Note that VROPlatformLoadBitmapFromAsset always generates RGBA8, even from
        // RGB8 images. This is considered optimal because sRGB8 is not compatible with
        // automatic mipmap generation in OpenGL 3.0
        _format = hasAlpha ? VROTextureFormat::RGBA8 : VROTextureFormat::RGB8;
        _internalFormat = VROTextureInternalFormat::RGBA8;
    }
}

VROImageAndroid::VROImageAndroid(jobject jbitmap, VROTextureInternalFormat internalFormat) {
    bool hasAlpha = false;
    _data = (unsigned char *)VROPlatformConvertBitmap(jbitmap, &_dataLength, &_width, &_height, &hasAlpha);
    if (internalFormat == VROTextureInternalFormat::RGB565) {
        _format = VROTextureFormat::RGB565;
        _internalFormat = VROTextureInternalFormat::RGB565;
    } else {
        // Internal format is always RGBA8, even for images that do not have alpha. This is because
        // sRGB8 is not compatible with automatic mipmap generation in OpenGL 3.0 (so we use sRGBA8).
        _format = hasAlpha ? VROTextureFormat::RGBA8 : VROTextureFormat::RGB8;
        _internalFormat = VROTextureInternalFormat::RGBA8;
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