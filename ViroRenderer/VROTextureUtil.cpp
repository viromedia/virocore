//
//  VROTextureUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/21/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROTextureUtil.h"
#include "VROTexture.h"
#include "VROByteBuffer.h"
#include "VROData.h"
#include "VROLog.h"

static const int kASTCHeaderLength = 16;
static const int kASTCBlockXOffset = 4;
static const int kASTCBlockYOffset = 5;
static const int kASTCWidthOffset  = 7;
static const int kASTCHeightOffset = 10;

static int read24BitInt(const uint8_t *buf) {
    return static_cast<int>(buf[0]) |
          (static_cast<int>(buf[1]) << 8) |
          (static_cast<int>(buf[2]) << 16);
}

std::shared_ptr<VROTexture> VROTextureUtil::loadASTCTexture(const uint8_t *data, int length, VROTextureType type,
                                                            VRODriver *driver) {
    int width;
    int height;
    VROTextureFormat format;
    std::shared_ptr<VROData> stripped = readASTCHeader(data, length, &format, &width, &height);
    
    return std::make_shared<VROTexture>(type, format, stripped, width, height, driver);
}

/*
 Read a texture file with a PKM header. Read the width and height from the header then
 strip it out and return the raw texture data.
 */
std::shared_ptr<VROData> VROTextureUtil::readASTCHeader(const uint8_t *data, int length, VROTextureFormat *outFormat,
                                                        int *outWidth, int *outHeight) {
    int blockDimX = data[kASTCBlockXOffset];
    int blockDimY = data[kASTCBlockYOffset];
    
    if (4 == blockDimX && 4 == blockDimY) {
        *outFormat = VROTextureFormat::ASTC_4x4_LDR;
    }
    else if (5 == blockDimX && 4 == blockDimY) {
        pabort();
    }
    else if (5 == blockDimX && 5 == blockDimY) {
        pabort();
    }
    else if (6 == blockDimX && 5 == blockDimY) {
        pabort();
    }
    else if (6 == blockDimX && 6 == blockDimY) {
        pabort();
    }
    else if (8 == blockDimX && 5 == blockDimY) {
        pabort();
    }
    else if (8 == blockDimX && 6 == blockDimY) {
        pabort();
    }
    else if (8 == blockDimX && 8 == blockDimY) {
        pabort();
    }
    else if (10 == blockDimX && 5 == blockDimY) {
        pabort();
    }
    else if (10 == blockDimX && 6 == blockDimY) {
        pabort();
    }
    else if (10 == blockDimX && 8 == blockDimY) {
        pabort();
    }
    else if (10 == blockDimX && 10 == blockDimY) {
        pabort();
    }
    else if (12 == blockDimX && 10 == blockDimY) {
        pabort();
    }
    else if (12 == blockDimX && 12 == blockDimY) {
        pabort();
    }
    else {
        // We don't support any other block dimensions..
        pabort();
    }
    
    *outWidth  = read24BitInt(data + kASTCWidthOffset);
    *outHeight = read24BitInt(data + kASTCHeightOffset);
    
    return std::make_shared<VROData>(((const char *) data) + kASTCHeaderLength,
                                     length - kASTCHeaderLength);
}