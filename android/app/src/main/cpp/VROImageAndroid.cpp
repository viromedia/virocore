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

VROImageAndroid::VROImageAndroid(jobject jbitmap) {
    _format = VROPlatformGetBitmapFormat(jbitmap);
    bool hasAlpha;
    _data = (unsigned char *)VROPlatformConvertBitmap(jbitmap, &_dataLength, &_width, &_height, &hasAlpha);
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

/*
 This function is used by VROARImageTargetAndroid (w/ ARCore). As such, we can
 make a few assumptions, that the data is from a Bitmap and is using the RGBA_8888
 format.
 */
unsigned char *VROImageAndroid::getGrayscaleData(size_t *length, size_t *stride) {
    *length = _dataLength;

    // we can make this assumption because VROPlatformConvertBitmap computes _dataLength = _stride * _height
    int32_t rgbastride = _dataLength / _height;
    *stride = rgbastride / 4;

    unsigned char *returnArr;
    convertRgbaToGrayscale(rgbastride, &returnArr);
    return returnArr;
}

/*
 This function comes from the augmented_image_c example provided by ARCore (util.h's
 ConvertRgbaToGrayscale function).
 */
void VROImageAndroid::convertRgbaToGrayscale(int32_t stride, uint8_t** out_grayscale_buffer) {
  int32_t grayscale_stride = stride / 4;  // Only support RGBA_8888 format
  uint8_t* grayscale_buffer = new uint8_t[grayscale_stride * _height];
  for (int h = 0; h < _height; ++h) {
    for (int w = 0; w < _width; ++w) {
      const uint8_t* pixel = &_data[w * 4 + h * stride];
      uint8_t r = *pixel;
      uint8_t g = *(pixel + 1);
      uint8_t b = *(pixel + 2);
      grayscale_buffer[w + h * grayscale_stride] =
          static_cast<uint8_t>(0.213f * r + 0.715 * g + 0.072 * b);
    }
  }
  *out_grayscale_buffer = grayscale_buffer;
}
