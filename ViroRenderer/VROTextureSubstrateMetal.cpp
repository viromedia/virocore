//
//  VROTextureSubstrateMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/4/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROTextureSubstrateMetal.h"
#include "VROImageUtil.h"
#include "VRORenderContextMetal.h"

VROTextureSubstrateMetal::VROTextureSubstrateMetal(UIImage *image,
                                                   const VRORenderContextMetal &context) {
    
    int width = image.size.width * image.scale;
    int height = image.size.height * image.scale;
    
    size_t dataLength;
    void *data = VROExtractRGBA8888FromImage(image, &dataLength);
    
    int bytesPerPixel = 4;
    MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                          width:width
                                                                                         height:height
                                                                                      mipmapped:NO];
    _texture = [context.getDevice() newTextureWithDescriptor:descriptor];
    
    MTLRegion region = MTLRegionMake2D(0, 0, width, height);
    [_texture replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:bytesPerPixel * width];
}

VROTextureSubstrateMetal::~VROTextureSubstrateMetal() {
    
}

