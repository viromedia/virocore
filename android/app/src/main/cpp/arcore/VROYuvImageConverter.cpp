//
//  VROYuvImageConverter.cpp
//  Viro
//
//  Created by Raj Advani on 4/9/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

// This file is modified from Image_Reader.cpp from
// https://github.com/sjfricke/OpenCV-NDK/blob/master/OpenCV-NDK/app/src/main/cpp/Image_Reader.cpp

/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "VROYuvImageConverter.h"
#include "ARCore_API.h"
#include "VROLog.h"
#include <algorithm>

static const int kMaxChannelValue = 262143;

static inline uint32_t YUV2RGB(int nY, int nU, int nV) {
    nY -= 16;
    nU -= 128;
    nV -= 128;
    if (nY < 0) nY = 0;

    // This is the floating point equivalent. We do the conversion in integer
    // because some Android devices do not have floating point in hardware.
    // nR = (int)(1.164 * nY + 1.596 * nV);
    // nG = (int)(1.164 * nY - 0.813 * nV - 0.391 * nU);
    // nB = (int)(1.164 * nY + 2.018 * nU);

    int nR = (int)(1192 * nY + 1634 * nV);
    int nG = (int)(1192 * nY - 833 * nV - 400 * nU);
    int nB = (int)(1192 * nY + 2066 * nU);

    nR = std::min(kMaxChannelValue, std::max(0, nR));
    nG = std::min(kMaxChannelValue, std::max(0, nG));
    nB = std::min(kMaxChannelValue, std::max(0, nB));

    nR = (nR >> 10) & 0xff;
    nG = (nG >> 10) & 0xff;
    nB = (nB >> 10) & 0xff;

    return 0xff000000 | (nR << 16) | (nG << 8) | nB;
}

void VROYuvImageConverter::convertImage(arcore::Image *image, uint8_t *data) {
    int left, right, bottom, top;
    image->getCropRect(&left, &right, &bottom, &top);

    int numPlanes = image->getNumberOfPlanes();
    if (numPlanes != 3) {
        pwarn("Cannot convert YCbCr image data to RGBA: detected %d planes instead of 3", numPlanes);
        return;
    }
    int32_t yStride  = image->getPlaneRowStride(0);
    int32_t uvStride = image->getPlaneRowStride(1);
    int32_t uvPixelStride = image->getPlanePixelStride(1);

    uint8_t *yPixel = nullptr;
    int32_t yLen;
    image->getPlaneData(0, &yPixel, &yLen);

    uint8_t *vPixel = nullptr;
    int32_t vLen;
    image->getPlaneData(1, &vPixel, &vLen);

    uint8_t *uPixel = nullptr;
    int32_t uLen;
    image->getPlaneData(2, &uPixel, &uLen);

    int32_t height = bottom - top;
    int32_t width = right - left;

    uint32_t *out = (uint32_t *) data;
    for (int32_t y = 0; y < height; y++) {
        const uint8_t *pY = yPixel + yStride * (y + top) + left;

        int32_t uv_row_start = uvStride * ((y + top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (left >> 1);

        for (int32_t x = 0; x < width; x++) {
            const int32_t uv_offset = (x >> 1) * uvPixelStride;
            out[x] = YUV2RGB(pY[x], pU[uv_offset], pV[uv_offset]);
        }
        out += width; // width is the int32_t stride
    }
}

void VROYuvImageConverter::convertImage90(arcore::Image *image, uint8_t *data) {
    int left, right, bottom, top;
    image->getCropRect(&left, &right, &bottom, &top);

    int numPlanes = image->getNumberOfPlanes();
    if (numPlanes != 3) {
        pwarn("Cannot convert YCbCr image data to RGBA: detected %d planes instead of 3", numPlanes);
        return;
    }
    int32_t yStride  = image->getPlaneRowStride(0);
    int32_t uvStride = image->getPlaneRowStride(1);
    int32_t uvPixelStride = image->getPlanePixelStride(1);

    uint8_t *yPixel = nullptr;
    int32_t yLen;
    image->getPlaneData(0, &yPixel, &yLen);

    uint8_t *vPixel = nullptr;
    int32_t vLen;
    image->getPlaneData(1, &vPixel, &vLen);

    uint8_t *uPixel = nullptr;
    int32_t uLen;
    image->getPlaneData(2, &uPixel, &uLen);

    int32_t height = bottom - top;
    int32_t width = right - left;

    uint32_t *out = (uint32_t *) data;
    out += height - 1;
    for (int32_t y = 0; y < height; y++) {
        const uint8_t *pY = yPixel + yStride * (y + top) + left;

        int32_t uv_row_start = uvStride * ((y + top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (left >> 1);

        for (int32_t x = 0; x < width; x++) {
            const int32_t uv_offset = (x >> 1) * uvPixelStride;
            // [x, y]--> [-y, x]
            int testb = pU[uv_offset];
            int testc = pV[uv_offset];
            int testA = pY[x];
            out[x * height] = YUV2RGB(testA, testb, testc);
        }
        out -= 1;  // move to the next column
    }
}

void VROYuvImageConverter::convertImage180(arcore::Image *image, uint8_t *data) {
    int left, right, bottom, top;
    image->getCropRect(&left, &right, &bottom, &top);

    int numPlanes = image->getNumberOfPlanes();
    if (numPlanes != 3) {
        pwarn("Cannot convert YCbCr image data to RGBA: detected %d planes instead of 3", numPlanes);
        return;
    }
    int32_t yStride  = image->getPlaneRowStride(0);
    int32_t uvStride = image->getPlaneRowStride(1);
    int32_t uvPixelStride = image->getPlanePixelStride(1);

    uint8_t *yPixel = nullptr;
    int32_t yLen;
    image->getPlaneData(0, &yPixel, &yLen);

    uint8_t *vPixel = nullptr;
    int32_t vLen;
    image->getPlaneData(1, &vPixel, &vLen);

    uint8_t *uPixel = nullptr;
    int32_t uLen;
    image->getPlaneData(2, &uPixel, &uLen);

    int32_t height = bottom - top;
    int32_t width = right - left;

    uint32_t *out = (uint32_t *) data;
    out += (height - 1) * width;
    for (int32_t y = 0; y < height; y++) {
        const uint8_t *pY = yPixel + yStride * (y + top) + left;

        int32_t uv_row_start = uvStride * ((y + top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (left >> 1);

        for (int32_t x = 0; x < width; x++) {
            const int32_t uv_offset = (x >> 1) * uvPixelStride;
            // mirror image since we are using front camera
            out[width - 1 - x] = YUV2RGB(pY[x], pU[uv_offset], pV[uv_offset]);
            // out[x] = YUV2RGB(pY[x], pU[uv_offset], pV[uv_offset]);
        }
        out -= width;
    }
}

void VROYuvImageConverter::convertImage270(arcore::Image *image, uint8_t *data) {
    int left, right, bottom, top;
    image->getCropRect(&left, &right, &bottom, &top);

    int numPlanes = image->getNumberOfPlanes();
    if (numPlanes != 3) {
        pwarn("Cannot convert YCbCr image data to RGBA: detected %d planes instead of 3", numPlanes);
        return;
    }
    int32_t yStride  = image->getPlaneRowStride(0);
    int32_t uvStride = image->getPlaneRowStride(1);
    int32_t uvPixelStride = image->getPlanePixelStride(1);

    uint8_t *yPixel = nullptr;
    int32_t yLen;
    image->getPlaneData(0, &yPixel, &yLen);

    uint8_t *vPixel = nullptr;
    int32_t vLen;
    image->getPlaneData(1, &vPixel, &vLen);

    uint8_t *uPixel = nullptr;
    int32_t uLen;
    image->getPlaneData(2, &uPixel, &uLen);

    int32_t height = bottom - top;
    int32_t width = right - left;

    uint32_t *out = (uint32_t *) data;
    for (int32_t y = 0; y < height; y++) {
        const uint8_t *pY = yPixel + yStride * (y + top) + left;

        int32_t uv_row_start = uvStride * ((y + top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (left >> 1);

        for (int32_t x = 0; x < width; x++) {
            const int32_t uv_offset = (x >> 1) * uvPixelStride;
            int testb = pU[uv_offset];
            int testc = pV[uv_offset];
            int testA = pY[x];
            out[(width - 1 - x) * height] = YUV2RGB(testA, testb, testc);
        }
        out += 1;  // move to the next column
    }
}