//
//  VROTextureUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/21/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

typedef struct {
    uint8_t m_au8Identifier[12];
    uint32_t m_u32Endianness;
    uint32_t m_u32GlType;
    uint32_t m_u32GlTypeSize;
    uint32_t m_u32GlFormat;
    uint32_t m_u32GlInternalFormat;
    uint32_t m_u32GlBaseInternalFormat;
    uint32_t m_u32PixelWidth;
    uint32_t m_u32PixelHeight;
    uint32_t m_u32PixelDepth;
    uint32_t m_u32NumberOfArrayElements;
    uint32_t m_u32NumberOfFaces;
    uint32_t m_u32NumberOfMipmapLevels;
    uint32_t m_u32BytesOfKeyValueData;
} VROKTXData;

std::shared_ptr<VROData> VROTextureUtil::readKTXHeader(const uint8_t *data, uint32_t length, VROTextureFormat *outFormat,
                                                       int *outWidth, int *outHeight, std::vector<uint32_t> *outMipSizes) {
    
    VROKTXData ktxHeader;
    memcpy(&ktxHeader, data, sizeof(VROKTXData));
    
    // We only support RGBA8 ETC2 currently
    passert (ktxHeader.m_u32GlInternalFormat == GL_COMPRESSED_RGBA8_ETC2_EAC);
    
    *outFormat = VROTextureFormat::ETC2_RGBA8_EAC;
    *outWidth  = ktxHeader.m_u32PixelWidth;
    *outHeight = ktxHeader.m_u32PixelHeight;
    
    int numMipLevels = ktxHeader.m_u32NumberOfMipmapLevels;
    
    // Skip key value pairs
    int mipStartIndex = sizeof(VROKTXData) + ktxHeader.m_u32BytesOfKeyValueData;
    
    VROByteBuffer buffer;
    
    for (int i = 0; i < numMipLevels; i++) {
        uint32_t mipSize;
        memcpy(&mipSize, ((const char *)data) + mipStartIndex, sizeof(uint32_t));
        
        outMipSizes->push_back(mipSize);
        
        mipStartIndex += sizeof(uint32_t);
        buffer.grow(mipSize);
        buffer.writeBytes(((const char *)data) + mipStartIndex, mipSize);
        
        uint32_t mipSizeRounded = (mipSize + 3) & ~(uint32_t)3;
        mipStartIndex += mipSizeRounded;
    }
    
    buffer.releaseBytes();
    return std::make_shared<VROData>(buffer.getData(), buffer.getPosition(), VRODataOwnership::Move);
}

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t numMipLevels;
} VROVHDData;

std::shared_ptr<VROData> VROTextureUtil::readVHDHeader(const std::string &data, VROTextureFormat *outFormat,
                                                       int *outWidth, int *outHeight, std::vector<uint32_t> *outMipSizes) {
    
    VROVHDData vhdHeader;
    memcpy(&vhdHeader, data.c_str(), sizeof(VROVHDData));
    
    *outFormat = VROTextureFormat::RGB9_E5;
    *outWidth  = vhdHeader.width;
    *outHeight = vhdHeader.height;
    
    int numMipLevels = vhdHeader.numMipLevels;
    
    int mipStartIndex = sizeof(VROVHDData);
    int mipWidth = vhdHeader.width;
    int mipHeight = vhdHeader.height;
    
    for (int i = 0; i < numMipLevels; i++) {
        uint32_t mipSize = mipWidth * mipHeight * sizeof(uint32_t);
        outMipSizes->push_back(mipSize);
        
        mipStartIndex += mipSize;
        mipWidth /= 2;
        mipHeight /= 2;
    }
    return std::make_shared<VROData>(data.c_str() + sizeof(VROVHDData), data.length() - sizeof(VROVHDData));
}

VROStereoMode VROTextureUtil::getStereoModeForString(std::string stereoModeTag){
    if (VROStringUtil::strcmpinsensitive(stereoModeTag, "LeftRight")){
        return VROStereoMode::LeftRight;
    } else if (VROStringUtil::strcmpinsensitive(stereoModeTag, "RightLeft")){
        return VROStereoMode::RightLeft;
    } else if (VROStringUtil::strcmpinsensitive(stereoModeTag, "TopBottom")){
        return VROStereoMode::TopBottom;
    } else if (VROStringUtil::strcmpinsensitive(stereoModeTag, "BottomTop")){
        return VROStereoMode::BottomTop;
    }

    return VROStereoMode::None;
}

