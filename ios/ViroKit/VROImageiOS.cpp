//
//  VROImageiOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/3/16.
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
