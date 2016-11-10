//
//  VROImageiOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/3/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROImageiOS.h"

VROImageiOS::VROImageiOS(UIImage *image) {
    CGImageRef imageRef = [image CGImage];
    _width = (int) CGImageGetWidth(imageRef);
    _height = (int) CGImageGetHeight(imageRef);
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    _dataLength = (_height * _width * 4 * sizeof(unsigned char));
    _data = (unsigned char *) calloc(_height * _width * 4, sizeof(unsigned char));
    
    NSUInteger bytesPerPixel = 4;
    NSUInteger bytesPerRow = bytesPerPixel * _width;
    NSUInteger bitsPerComponent = 8;
    
    CGContextRef context = CGBitmapContextCreate(_data, _width, _height,
                                                 bitsPerComponent, bytesPerRow, colorSpace,
                                                 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(colorSpace);
    CGContextDrawImage(context, CGRectMake(0, 0, _width, _height), imageRef);
    CGContextRelease(context);
}

VROImageiOS::~VROImageiOS() {
    free (_data);
}

unsigned char *VROImageiOS::extractRGBA8888(size_t *length) {
    *length = _dataLength;
    return _data;
}
