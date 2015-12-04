//
//  VROMetalUtils.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/30/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMetalUtils.h"
#include "VROVector4f.h"
#include "VROMatrix4f.h"
#include "VROImageUtil.h"

static id <MTLTexture> staticBlankTexture = nullptr;

vector_float4 toVectorFloat4(VROVector4f v) {
    return { v.x, v.y, v.z, v.w };
}

matrix_float4x4 toMatrixFloat4x4(VROMatrix4f m) {
    matrix_float4x4 m4x4 = {
        .columns[0] = { m[0],  m[1],  m[2],  m[3]  },
        .columns[1] = { m[4],  m[5],  m[6],  m[7]  },
        .columns[2] = { m[8],  m[9],  m[10], m[11] },
        .columns[3] = { m[12], m[13], m[14], m[15] }
    };
    
    return m4x4;
}

id <MTLTexture> getBlankTexture(id <MTLDevice> device) {
    if (!staticBlankTexture) {
        UIImage *image = [UIImage imageNamed:@"blank"];
        int width = image.size.width;
        int height = image.size.height;
        
        size_t dataLength;
        void *data = VROExtractRGBA8888FromImage(image, &dataLength);
 
        int bytesPerPixel = 4;
        MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                              width:width
                                                                                             height:height
                                                                                          mipmapped:NO];
        staticBlankTexture = [device newTextureWithDescriptor:descriptor];
        
        MTLRegion region = MTLRegionMake2D(0, 0, width, height);
        [staticBlankTexture replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:bytesPerPixel * width];
    }
    
    return staticBlankTexture;
}

