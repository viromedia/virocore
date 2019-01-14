//
//  VROImagePreprocessor.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/14/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#import "VROImagePreprocessor.h"
#import <Accelerate/Accelerate.h>
#import <UIKit/UIKit.h>
#include "Endian.h"
#include "VROLog.h"

#define clamp(a) (a > 255 ? 255 : (a < 0 ? 0 : a));

void stillImageDataReleaseCallback(void *releaseRefCon, const void *baseAddress) {
    free((void *)baseAddress);
}

CVPixelBufferRef VROImagePreprocessor::rotateImage(CVPixelBufferRef image, uint8_t rotation) {
    CVPixelBufferLockBaseAddress(image, 0);
    
    size_t width = CVPixelBufferGetWidth(image);
    size_t height = CVPixelBufferGetHeight(image);
    
    size_t resultWidth = width;
    size_t resultHeight = height;
    if (rotation == 1 || rotation == 3) {
        resultWidth = height;
        resultHeight = width;
    }
    
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(image);
    size_t bytesPerRowOut = 4 * resultWidth * sizeof(unsigned char);
    size_t currSize = bytesPerRow * height * sizeof(unsigned char);
    
    void *srcBuff = CVPixelBufferGetBaseAddress(image);
    unsigned char *outBuff = (unsigned char *) malloc(currSize);
    
    vImage_Buffer ibuff = { srcBuff, height, width, bytesPerRow};
    vImage_Buffer ubuff = { outBuff, resultHeight, resultWidth, bytesPerRowOut};
    uint8_t bgColor[4]  = {0, 0, 0, 0};
    
    // For the rotation parameter [0, 1, 2, 3] map to [0, 90, 180, 270] degree rotation
    vImage_Error err = vImageRotate90_ARGB8888(&ibuff, &ubuff, rotation, bgColor, 0);
    if (err != kvImageNoError) {
        pinfo("Error performing image rotation");
    }
    CVPixelBufferUnlockBaseAddress(image, 0);
    
    CVPixelBufferRef rotatedBuffer = NULL;
    CVPixelBufferCreateWithBytes(NULL,
                                 resultWidth,
                                 resultHeight,
                                 kCVPixelFormatType_32BGRA,
                                 ubuff.data,
                                 bytesPerRowOut,
                                 stillImageDataReleaseCallback,
                                 NULL,
                                 NULL,
                                 &rotatedBuffer);
    return rotatedBuffer;
}

CVPixelBufferRef VROImagePreprocessor::convertYCbCrToRGB(CVImageBufferRef imageBuffer) {
    CVPixelBufferLockBaseAddress(imageBuffer, 0);
    
    size_t width = CVPixelBufferGetWidth(imageBuffer);
    size_t height = CVPixelBufferGetHeight(imageBuffer);
    
    uint8_t *baseAddress = (uint8_t *)CVPixelBufferGetBaseAddress(imageBuffer);
    CVPlanarPixelBufferInfo_YCbCrBiPlanar *bufferInfo = (CVPlanarPixelBufferInfo_YCbCrBiPlanar *)baseAddress;
    
    NSUInteger yOffset = EndianU32_BtoN(bufferInfo->componentInfoY.offset);
    NSUInteger yPitch = EndianU32_BtoN(bufferInfo->componentInfoY.rowBytes);
    
    NSUInteger cbCrOffset = EndianU32_BtoN(bufferInfo->componentInfoCbCr.offset);
    NSUInteger cbCrPitch = EndianU32_BtoN(bufferInfo->componentInfoCbCr.rowBytes);
    int bytesPerPixel = 4;
    uint8_t *rgbBuffer = (uint8_t *) malloc(width * height * bytesPerPixel);
    uint8_t *yBuffer = baseAddress + yOffset;
    uint8_t *cbCrBuffer = baseAddress + cbCrOffset;
    
    for (int y = 0; y < height; y++) {
        uint8_t *rgbBufferLine = &rgbBuffer[y * width * bytesPerPixel];
        uint8_t *yBufferLine = &yBuffer[y * yPitch];
        uint8_t *cbCrBufferLine = &cbCrBuffer[(y >> 1) * cbCrPitch];
        
        for (int x = 0; x < width; x++) {
            // from ITU-R BT.601, rounded to integers
            int16_t y = yBufferLine[x] - 16;
            int16_t cb = cbCrBufferLine[x & ~1] - 128;
            int16_t cr = cbCrBufferLine[x | 1] - 128;
            
            uint8_t *rgbOutput = &rgbBufferLine[x * bytesPerPixel];
            
            int16_t r = (int16_t)roundf( y + cr * 1.4 );
            int16_t g = (int16_t)roundf( y + cb * -0.343 + cr * -0.711 );
            int16_t b = (int16_t)roundf( y + cb *  1.765);
            
            //int16_t r = (int16_t)roundf( y + cb *  0.0    + cr *  1.4020 - 0.7010);
            //int16_t g = (int16_t)roundf( y + cb * -0.3441 + cr * -0.7141 + 0.5291);
            //int16_t b = (int16_t)roundf( y + cb *  1.7720 + cr *  0.0    - 0.8660);
            
            //BGRA
            rgbOutput[0] = clamp(b);
            rgbOutput[1] = clamp(g);
            rgbOutput[2] = clamp(r);
            rgbOutput[3] = 0xFF;
        }
    }
    
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    CVPixelBufferRef pixel_buffer = NULL;
    CVPixelBufferCreateWithBytes(kCFAllocatorDefault, width, height, kCVPixelFormatType_32BGRA, rgbBuffer,
                                 width * 4, stillImageDataReleaseCallback, NULL, NULL, &pixel_buffer);
    return pixel_buffer;
}

void VROImagePreprocessor::writeImageToPhotos(CVPixelBufferRef image) {
    CVPixelBufferLockBaseAddress(image, 0);
    Byte *rawImageBytes = (Byte *) CVPixelBufferGetBaseAddress(image);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(image);
    size_t width = CVPixelBufferGetWidth(image);
    size_t height = CVPixelBufferGetHeight(image);
    
    // Create suitable color space
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    // Create a suitable CGContext (suitable for camera output setting kCVPixelFormatType_32BGRA)
    CGContextRef newContext = CGBitmapContextCreate(rawImageBytes, width, height, 8, bytesPerRow, colorSpace,
                                                    kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst);
    CVPixelBufferUnlockBaseAddress(image, 0);
    CGColorSpaceRelease(colorSpace);
    
    // Create a CGImage from the CFContext, then a UIImage from the CGImage
    CGImageRef newImage = CGBitmapContextCreateImage(newContext);
    UIImage *finalImage = [[UIImage alloc] initWithCGImage:newImage];
    UIImageWriteToSavedPhotosAlbum(finalImage, nil, nil, nil);
}
