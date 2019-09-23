//
//  VROImagePreprocessor.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/14/19.
//  Copyright © 2019 Viro Media. All rights reserved.
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

#import "VROImagePreprocessor.h"
#import <Accelerate/Accelerate.h>
#import <UIKit/UIKit.h>
#include "Endian.h"
#include "VROLog.h"

#define clamp(a) (a > 255 ? 255 : (a < 0 ? 0 : a));

void stillImageDataReleaseCallback(void *releaseRefCon, const void *baseAddress) {
    free((void *)baseAddress);
}

CVPixelBufferRef VROImagePreprocessor::rotateImage(CVPixelBufferRef image, uint8_t rotation,
                                                   size_t *outResultWidth, size_t *outResultHeight) {
    CVPixelBufferLockBaseAddress(image, 0);
    
    size_t width = CVPixelBufferGetWidth(image);
    size_t height = CVPixelBufferGetHeight(image);
    
    if (rotation == 1 || rotation == 3) {
        *outResultWidth = height;
        *outResultHeight = width;
    } else {
        *outResultWidth = width;
        *outResultHeight = height;
    }
    
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(image);
    size_t bytesPerRowOut = 4 * *outResultWidth * sizeof(unsigned char);
    size_t currSize = bytesPerRow * height * sizeof(unsigned char);
    
    void *srcBuff = CVPixelBufferGetBaseAddress(image);
    unsigned char *outBuff = (unsigned char *) malloc(currSize);
    
    vImage_Buffer ibuff = { srcBuff, height, width, bytesPerRow};
    vImage_Buffer ubuff = { outBuff, *outResultHeight, *outResultWidth, bytesPerRowOut};
    uint8_t bgColor[4]  = {0, 0, 0, 0};
    
    // For the rotation parameter [0, 1, 2, 3] map to [0, 90, 180, 270] degree rotation
    vImage_Error err = vImageRotate90_ARGB8888(&ibuff, &ubuff, rotation, bgColor, 0);
    if (err != kvImageNoError) {
        pinfo("Error performing image rotation");
    }
    CVPixelBufferUnlockBaseAddress(image, 0);
    
    CVPixelBufferRef rotatedBuffer = NULL;
    CVPixelBufferCreateWithBytes(NULL,
                                 *outResultWidth,
                                 *outResultHeight,
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

CVPixelBufferRef VROImagePreprocessor::cropAndResize(CVPixelBufferRef pixelBuffer, int cropX, int cropY,
                                                     int cropWidth, int cropHeight, int size, uint8_t *scratchBuffer) {
    CVPixelBufferLockBaseAddress(pixelBuffer, 0);
    
    uint8_t *baseAddress = (uint8_t *)CVPixelBufferGetBaseAddress(pixelBuffer);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);
    size_t offset = cropY * bytesPerRow + cropX * 4;
    
    /*
     Create a buffer wrapping the original CVPixelBuffer, representing the cropped image.
     */
    vImage_Buffer cropped;
    cropped.data = baseAddress + offset;
    cropped.height = (vImagePixelCount) cropHeight;
    cropped.width = (vImagePixelCount) cropWidth;
    cropped.rowBytes = bytesPerRow;
    
    /*
     Allocate new space for the resized image.
     */
    size_t resizedBytesPerRow = size * 4;
    uint8_t *resizedData = (uint8_t *)malloc(size * resizedBytesPerRow);
    
    /*
     Create a buffer representing the resized image.
     */
    vImage_Buffer resized;
    resized.data = resizedData;
    resized.height = (vImagePixelCount) size;
    resized.width = (vImagePixelCount) size;
    resized.rowBytes = resizedBytesPerRow;
    
    /*
     Derive the scale transform required to maintain aspect ratio (fit the long
     side, and pad the short side into the center).
     */
    float scale = (float) size / (float) fmax(cropWidth, cropHeight);
    vImage_AffineTransform transform;
    transform.a = scale;
    transform.b = 0;
    transform.c = 0;
    transform.d = scale;
    if (cropWidth > cropHeight) {
        // Add top and bottom bars
        transform.tx = 0;
        transform.ty = scale * (cropWidth - cropHeight) / 2.0;
    } else {
        // Add left and right bars
        transform.tx = scale * (cropHeight - cropWidth) / 2.0;
        transform.ty = 0;
    }
    
    /*
     Perform the warp operation using the transform, filling the background with grey.
     */
    Pixel_8888 backgroundColor = { 128, 128, 128, 255 };
    vImageAffineWarp_ARGB8888(&cropped, &resized, scratchBuffer, &transform, backgroundColor, kvImageBackgroundColorFill);
    
    /*
     Create a new CVPixelBuffer out of the resized vImage.
     */
    OSType pixelFormat = CVPixelBufferGetPixelFormatType(pixelBuffer);
    CVPixelBufferRef resizedPixelBuffer;
    int status = CVPixelBufferCreateWithBytes(kCFAllocatorDefault, size, size, pixelFormat, resized.data,
                                              resized.rowBytes, stillImageDataReleaseCallback,
                                              nil, nil, &resizedPixelBuffer);
    
    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

    if (status != kCVReturnSuccess) {
        pinfo("Error: could not create new pixel buffer");
        return nil;
    }
    return resizedPixelBuffer;
}

CVPixelBufferRef VROImagePreprocessor::cropAndResizeCI(CVPixelBufferRef pixelBuffer, int cropX, int cropY,
                                                       int cropWidth, int cropHeight, int size) {
    CIImage *image = [[CIImage alloc] initWithCVPixelBuffer:pixelBuffer];
    image = [image imageByCroppingToRect:CGRectMake(cropX, cropY, cropWidth, cropHeight)];
    
    //float longSide = fmax(cropWidth, cropHeight);
    //float scale = size / longSide;
    
    //CGAffineTransform transform = CGAffineTransformMakeScale(size / CVPixelBufferGetWidth(pixelBuffer), size / CVPixelBufferGetHeight(pixelBuffer));
    //image = [image imageByApplyingTransform:transform];
    
    //CIImage *paddingColor = [[CIImage alloc] initWithColor:[CIColor grayColor]];
    
    CIContext *context = [[CIContext alloc] init];
    CVPixelBufferRef resizedPixelBuffer;
    CVPixelBufferCreate(kCFAllocatorDefault, cropWidth, cropHeight, CVPixelBufferGetPixelFormatType(pixelBuffer), nil, &resizedPixelBuffer);
    [context render:image toCVPixelBuffer:resizedPixelBuffer];
    
    return resizedPixelBuffer;
}
