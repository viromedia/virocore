//
//  VROImageMacOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/3/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROImageMacOS.h"
#import <Accelerate/Accelerate.h>
#include "VROLog.h"
#include "VROTime.h"

VROImageMacOS::VROImageMacOS(NSImage *image, VROTextureInternalFormat internalFormat) {
    passert (image != nullptr);
    
    NSRect imageRect = NSMakeRect(0, 0, image.size.width, image.size.height);
    CGImageRef imageRef = [image CGImageForProposedRect:&imageRect context:NULL hints:nil];
    
    _width = (int) CGImageGetWidth(imageRef);
    _height = (int) CGImageGetHeight(imageRef);
    
    if (internalFormat == VROTextureInternalFormat::RGB565) {
        NSUInteger bytesPerPixel = 2;
        
        vImage_Buffer srcBuffer;
        srcBuffer.width = _width;
        srcBuffer.height = _height;
        srcBuffer.rowBytes = bytesPerPixel * _width;
        srcBuffer.data = (unsigned char *) calloc(_height * srcBuffer.rowBytes, sizeof(unsigned char));

        vImage_CGImageFormat format = {
            .bitsPerComponent = 5,
            .bitsPerPixel = 16,
            .colorSpace = NULL,
            .bitmapInfo = kCGImageAlphaNone | kCGBitmapByteOrder16Little,
            .version = 0,
            .decode = NULL,
            .renderingIntent = kCGRenderingIntentDefault,
        };
        vImage_Error ret = vImageBuffer_InitWithCGImage(&srcBuffer, &format, NULL, imageRef, kvImageNoAllocate);
        passert (ret == kvImageNoError);
        
        _dataLength = _height * _width * (int)bytesPerPixel * sizeof(unsigned char);
        _data = (unsigned char *)srcBuffer.data;
        _format = VROTextureFormat::RGB565;
        _internalFormat = VROTextureInternalFormat::RGB565;
    }
    
    // RGBA
    else {
        NSUInteger bytesPerPixel = 4;
        
        vImage_Buffer srcBuffer;
        srcBuffer.width = _width;
        srcBuffer.height = _height;
        srcBuffer.rowBytes = bytesPerPixel * _width;
        srcBuffer.data = (unsigned char *) calloc(_height * srcBuffer.rowBytes, sizeof(unsigned char));
        
        vImage_CGImageFormat format = {
            .bitsPerComponent = 8,
            .bitsPerPixel = 32,
            .colorSpace = NULL,
            .bitmapInfo = (CGBitmapInfo)kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big,
            .version = 0,
            .decode = NULL,
            .renderingIntent = kCGRenderingIntentDefault,
        };
        
        vImage_Error ret = vImageBuffer_InitWithCGImage(&srcBuffer, &format, NULL, imageRef, kvImageNoAllocate);
        passert (ret == kvImageNoError);
        
        _dataLength = _height * _width * (int)bytesPerPixel * sizeof(unsigned char);
        _data = (unsigned char *)srcBuffer.data;
        
        if (hasAlpha(image)) {
            _format = VROTextureFormat::RGBA8;
            _internalFormat = VROTextureInternalFormat::RGBA8;
        }
        else {
            // Note that the internal format remains as RGBA8, because we do not provide an internal
            // RGB8 format (because sRGB8 is not compatible with automatic mipmap generation in
            // OpenGL 3.0).
            _format = VROTextureFormat::RGB8;
            _internalFormat = VROTextureInternalFormat::RGBA8;
        }
    }
}

VROImageMacOS::~VROImageMacOS() {
    free (_data);
}

unsigned char *VROImageMacOS::getData(size_t *length) {
    *length = _dataLength;
    return _data;
}

bool VROImageMacOS::hasAlpha(NSImage *image) {
    return true;
}
