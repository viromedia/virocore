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

static const int kASTCHeaderLength = 16;
static const int kASTCWidthOffset  = 7;
static const int kASTCHeightOffset = 10;

static int read24BitInt(const char *in) {
    unsigned int val=0;
    val = val | in[0];
    val = val << 8;
    val = val | in[1];
    val = val << 8;
    val = val | in[2];
    
    return val;
}

std::shared_ptr<VROTexture> VROTextureUtil::loadASTCTexture(NSData *data, VROTextureType type, VROTextureFormat format,
                                                            const VRORenderContext *context) {
    int width;
    int height;
    std::shared_ptr<VROData> stripped = readASTCHeader(data, &width, &height);
    
    return std::make_shared<VROTexture>(type, format, stripped, width, height, false, context);
}

/*
 Read a texture file with a PKM header. Read the width and height from the header then
 strip it out and return the raw texture data.
 */
std::shared_ptr<VROData> VROTextureUtil::readASTCHeader(NSData *data, int *width, int *height) {
    *width  = read24BitInt(((char *)[data bytes]) + kASTCWidthOffset);
    *height = read24BitInt(((char *)[data bytes]) + kASTCHeightOffset);
    
    return std::make_shared<VROData>(((char*) [data bytes]) + kASTCHeaderLength,
                                     [data length] - kASTCHeaderLength);
}