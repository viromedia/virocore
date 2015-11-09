//
//  VROImageUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/21/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROImageUtil.h"
#include "VROLog.h"

unsigned char *VROExtractRGBA8888FromImage(UIImage *image, size_t *length) {
    CGImageRef imageRef = [image CGImage];
    NSUInteger width = CGImageGetWidth(imageRef);
    NSUInteger height = CGImageGetHeight(imageRef);
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    *length = (height * width * 4 * sizeof(unsigned char));
    unsigned char *rawData = (unsigned char *) calloc(height * width * 4, sizeof(unsigned char));
    
    NSUInteger bytesPerPixel = 4;
    NSUInteger bytesPerRow = bytesPerPixel * width;
    NSUInteger bitsPerComponent = 8;
    
    CGContextRef context = CGBitmapContextCreate(rawData, width, height,
                                                 bitsPerComponent, bytesPerRow, colorSpace,
                                                 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(colorSpace);
    
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
    CGContextRelease(context);
    
    return rawData;
}

void *VROImageLoadTextureDataRGBA8888(const char *resource, size_t *bitmapLength, int *width, int *height) {
    NSString *file = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:resource] ofType:@"png"];
    UIImage *image = [UIImage imageWithContentsOfFile:file];
    
    passert (image != nullptr);
    
    *width = [image size].width;
    *height = [image size].height;
    
    return VROExtractRGBA8888FromImage(image, bitmapLength);
}