//
//  VROARBodyMeshingPoints.m
//  ViroKit
//
//  Created by vik.advani on 9/4/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROBodyTrackeriOS.h"
#include "VROLog.h"
#include "Endian.h"
#import "model_cpm.h"
#import <UIKit/UIKit.h>
#include <Accelerate/Accelerate.h>

#define clamp(a) (a>255?255:(a<0?0:a));

@interface NSArray (Map)

- (NSArray *)mapObjectsUsingBlock:(id (^)(id obj, NSUInteger idx))block;

@end

@implementation NSArray (Map)

- (NSArray *)mapObjectsUsingBlock:(id (^)(id obj, NSUInteger idx))block {
    NSMutableArray *result = [NSMutableArray arrayWithCapacity:[self count]];
    [self enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [result addObject:block(obj, idx)];
    }];
    return result;
}

@end

@implementation BodyPointImpl

@end

VROBodyTrackeriOS::VROBodyTrackeriOS() {
    _fps = 15;
}

bool VROBodyTrackeriOS::initBodyTracking(VROCameraPosition position,
                                     std::shared_ptr<VRODriver> driver) {
    
    _model = [[[model_cpm alloc] init] model];
    _coreMLModel =  [VNCoreMLModel modelForMLModel: _model error:nil];
    _coreMLRequest = [[VNCoreMLRequest alloc] initWithModel:_coreMLModel
                                          completionHandler:(VNRequestCompletionHandler) ^(VNRequest *request, NSError *error) {
        dispatch_async(dispatch_get_main_queue(), ^{
            NSArray *array = [request results];
            VNCoreMLFeatureValueObservation *topResult = (VNCoreMLFeatureValueObservation *)(array[0]);
            MLMultiArray *heatmap = topResult.featureValue.multiArrayValue;
            NSDictionary *k_dPoints = convert(heatmap);
            
            std::shared_ptr<VROBodyTrackerDelegate> delegate = _bodyMeshDelegateWeak.lock();
            if (delegate) {
                delegate->onBodyJointsFound(k_dPoints);
            }
        });
    }];
    
    _coreMLRequest.imageCropAndScaleOption = VNImageCropAndScaleOptionScaleFill;
    return true;
}

void VROBodyTrackeriOS::printBodyPoint(NSDictionary *bodyPoints, VROBodyMeshingJoints jointType) {
    NSNumber *index = [NSNumber numberWithInt:static_cast<int>(jointType)];
    if(bodyPoints[index] == [NSNull null]) {
        NSLog(@"Joint type %d is NULL", jointType);
    } else {
        NSValue *bodyPointValue = (NSValue *)bodyPoints[index];
        if(bodyPointValue != nil) {
            BodyPoint *bodyPoint = (BodyPoint *)bodyPointValue.pointerValue;
            float x= bodyPoint->_point.x;
            float y = bodyPoint->_point.y;
            float confidence = bodyPoint->_confidence;
            NSLog(@"Joint %d, values x,y:(%f, %f), confidence: %f", jointType, x,y, confidence);
        }
    }
}

NSDictionary *VROBodyTrackeriOS::convert(MLMultiArray *heatmap) {
    if(heatmap.shape.count < 3) {
        //print("heatmap's shape is invalid. \(heatmap.shape)")
        return nil;// nullptr;
    }
   
    NSInteger keypoint_number = heatmap.shape[0].integerValue;
    NSInteger heatmap_w = heatmap.shape[1].integerValue;
    NSInteger heatmap_h = heatmap.shape[2].integerValue;
 
    NSMutableDictionary *n_kpoints = [[NSMutableDictionary alloc] init];
   
    for (int k=0; k<keypoint_number; k++) {
        NSNumber *keyIndex = [NSNumber numberWithInt:k];
        [n_kpoints setObject:[NSNull null] forKey:keyIndex];
    }
    
    for (int k=0; k<keypoint_number; k++) {
        for(int i=0; i<heatmap_w; i++) {
            for(int j=0; j<heatmap_h; j++) {
                long index = k*(heatmap_w*heatmap_h) + i*(heatmap_h) + j;
                double confidence = heatmap[index].doubleValue;
                if(confidence > 0.5) {
                    NSLog(@"ChatH");
                }
                NSNumber *keyIndex = [NSNumber numberWithInt:k];
                if(confidence > 0) {
                   if (n_kpoints[keyIndex] == [NSNull null] ||
                       (n_kpoints[keyIndex] != [NSNull null] && isBodyPointConfidenceLessThan((BodyPointImpl *)n_kpoints[keyIndex], confidence))) {
                           CGPoint cgPoint;
                           cgPoint.x = CGFloat(j);
                           cgPoint.y = CGFloat(i);
                            BodyPointImpl *bodyImpl = [[BodyPointImpl alloc] init];
                            bodyImpl._point = cgPoint;
                            bodyImpl._confidence = confidence;
                           [n_kpoints setObject:bodyImpl forKey:keyIndex];
                        }
                } else {
                    continue;
                }
           }
       }
   }
   
   // transpose to (1.0, 1.0)
    for(id key in n_kpoints) {
        id value = [n_kpoints objectForKey:key];
        if(value == [NSNull null]) {
            
        } else {
            BodyPointImpl *bodyImpl = (BodyPointImpl *)value;
            CGFloat x = (bodyImpl._point.x + 0.5)/CGFloat(heatmap_w);
            CGFloat y = (bodyImpl._point.y + 0.5)/CGFloat(heatmap_h);
            CGPoint newPoint;
            newPoint.x = x;
            newPoint.y = y;
            bodyImpl._point = newPoint;
        }
    }
    
    NSDictionary *dict = [NSDictionary dictionaryWithDictionary:n_kpoints];
    return dict;
}
                       
bool VROBodyTrackeriOS::isBodyPointConfidenceLessThan(BodyPointImpl *bodyPoint, float confidence) {
    if(bodyPoint._confidence < confidence) {
        return true;
    } else {
        return false;
    }
}

void VROBodyTrackeriOS::startBodyTracking() {
    
}

void VROBodyTrackeriOS::stopBodyTracking() {
    
}

void stillImageDataReleaseCallback(void *releaseRefCon, const void *baseAddress)
{
    free((void *)baseAddress);
}

void VROBodyTrackeriOS::processBuffer(CVPixelBufferRef sampleBuffer) {
    NSLog(@"CaptureOutput invoked");
    //CMTime timestamp = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
    CMTime timestamp = CMClockGetTime(CMClockGetHostTimeClock());
    CMTime deltaTime = CMTimeSubtract(timestamp, _lastTimestamp);
    //format is equal to '420f'.
    OSType format = CVPixelBufferGetPixelFormatType(sampleBuffer);
    
    if (CMTimeCompare(deltaTime, CMTimeMake(1, _fps))) {
        NSLog(@"Running frame!!");
        _lastTimestamp = timestamp;
        CVPixelBufferRef convertedImage = convertImage(sampleBuffer);
        CVPixelBufferLockBaseAddress(convertedImage, 0);
       
        //rotate image
        size_t bytesPerRow = CVPixelBufferGetBytesPerRow(convertedImage);
        size_t width = CVPixelBufferGetWidth(convertedImage);
        size_t height = CVPixelBufferGetHeight(convertedImage);
        size_t currSize = bytesPerRow*height*sizeof(unsigned char);
        size_t bytesPerRowOut = 4*height*sizeof(unsigned char);

        void *srcBuff = CVPixelBufferGetBaseAddress(convertedImage);
        unsigned char *outBuff = (unsigned char*)malloc(currSize);
        
        size_t outputHeight = width;
        size_t outputWidth = height;
        vImage_Buffer ibuff = { srcBuff, height, width, bytesPerRow};
        vImage_Buffer ubuff = { outBuff, outputHeight, outputWidth, bytesPerRowOut};
        
        uint8_t rotConst = 3;   // 0, 1, 2, 3 is equal to 0, 90, 180, 270 degrees rotation
        
        uint8_t bgColor[4]  = {0, 0, 0, 0};
        vImage_Error err= vImageRotate90_ARGB8888(&ibuff, &ubuff, rotConst, bgColor, 0);
        CVPixelBufferUnlockBaseAddress(convertedImage, 0);
        
        
        rotatedBuffer = NULL;
        CVPixelBufferCreateWithBytes(NULL,
                                     outputWidth,
                                     outputHeight,
                                     kCVPixelFormatType_32BGRA,
                                     ubuff.data,
                                     bytesPerRowOut,
                                     stillImageDataReleaseCallback,
                                     NULL,
                                     NULL,
                                     &rotatedBuffer);
        CVPixelBufferRelease(convertedImage);
        //writeImageToDisk(rotatedBuffer);
        VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCVPixelBuffer:rotatedBuffer options:nil];
        dispatch_async(dispatch_get_main_queue(), ^{
            [handler performRequests:@[_coreMLRequest] error:nil];
        });
        CVPixelBufferRelease(rotatedBuffer);
    }
}

void VROBodyTrackeriOS::writeImageToDisk(CVPixelBufferRef imageBuffer) {

    CVPixelBufferLockBaseAddress(imageBuffer, 0);
    Byte *rawImageBytes = (Byte *)CVPixelBufferGetBaseAddress(imageBuffer);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer);
    size_t width = CVPixelBufferGetWidth(imageBuffer);
    size_t height = CVPixelBufferGetHeight(imageBuffer);
    NSData *dataForRawBytes = [NSData dataWithBytes:rawImageBytes length:bytesPerRow * CVPixelBufferGetHeight(imageBuffer)];
    // Do whatever with your bytes

    // create suitable color space
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

    //Create suitable context (suitable for camera output setting kCVPixelFormatType_32BGRA)
    CGContextRef newContext = CGBitmapContextCreate(rawImageBytes, width, height, 8, bytesPerRow, colorSpace, kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst);
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);

    // release color space
    CGColorSpaceRelease(colorSpace);

    //Create a CGImageRef from the CVImageBufferRef
    CGImageRef newImage = CGBitmapContextCreateImage(newContext);
    UIImage *FinalImage = [[UIImage alloc] initWithCGImage:newImage];
     UIImageWriteToSavedPhotosAlbum(FinalImage, nil, nil, nil);
}

static const int kMaxChannelValue = 262143;

CVPixelBufferRef VROBodyTrackeriOS::convertImage(CVImageBufferRef imageBuffer) {
    
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
    uint8_t *rgbBuffer = (uint8_t *)malloc(width * height * bytesPerPixel);
    uint8_t *yBuffer = baseAddress + yOffset;
    uint8_t *cbCrBuffer = baseAddress + cbCrOffset;
    
    for(int y = 0; y < height; y++)
    {
        uint8_t *rgbBufferLine = &rgbBuffer[y * width * bytesPerPixel];
        uint8_t *yBufferLine = &yBuffer[y * yPitch];
        uint8_t *cbCrBufferLine = &cbCrBuffer[(y >> 1) * cbCrPitch];
        
        for(int x = 0; x < width; x++)
        {
            // from ITU-R BT.601, rounded to integers
            int16_t y = yBufferLine[x] - 16;
            int16_t cb = cbCrBufferLine[x & ~1] - 128;
            int16_t cr = cbCrBufferLine[x | 1] - 128;
            
            uint8_t *rgbOutput = &rgbBufferLine[x*bytesPerPixel];
            int16_t r = (int16_t)roundf( y + cr *  1.4 );
            int16_t g = (int16_t)roundf( y + cb * -0.343 + cr * -0.711 );
            int16_t b = (int16_t)roundf( y + cb *  1.765);
            //BGRA
             rgbOutput[0] = clamp(b);
             rgbOutput[1] = clamp(g);
             rgbOutput[2] = clamp(r);
             rgbOutput[3] = 0xFF;
          }
    }
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    CVPixelBufferRef pixel_buffer = NULL;
    CVPixelBufferCreateWithBytes(kCFAllocatorDefault, width, height, kCVPixelFormatType_32BGRA, rgbBuffer, width * 4, stillImageDataReleaseCallback, NULL, NULL, &pixel_buffer);
    return pixel_buffer;
}

