//
//  VROImageiOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/3/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROImageiOS.h"
#import <Accelerate/Accelerate.h>
#include "VROLog.h"
#include "VROTime.h"

VROImageiOS::VROImageiOS(UIImage *image, VROTextureInternalFormat internalFormat) {
    CGImageRef imageRef = [image CGImage];
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
        _format = hasAlpha(image) ? VROTextureFormat::RGBA8 : VROTextureFormat::RGB8;
    }
}

VROImageiOS::~VROImageiOS() {
    free (_data);
}

unsigned char *VROImageiOS::getData(size_t *length) {
    *length = _dataLength;
    return _data;
}

bool VROImageiOS::hasAlpha(UIImage *image) {
    CGImageAlphaInfo alpha = CGImageGetAlphaInfo(image.CGImage);
    return (alpha == kCGImageAlphaFirst ||
            alpha == kCGImageAlphaLast ||
            alpha == kCGImageAlphaPremultipliedFirst ||
            alpha == kCGImageAlphaPremultipliedLast);
}
