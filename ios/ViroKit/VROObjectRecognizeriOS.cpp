//
//  VROObjectRecognizeriOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/10/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROObjectRecognizeriOS.h"
#include "VROLog.h"
#include "VROTime.h"
#import "model_pipelined.h"
#import "VROImagePreprocessor.h"
#import "VRODriverOpenGLiOS.h"

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

VROObjectRecognizeriOS::VROObjectRecognizeriOS() {
    _fps = 0;
    _lastTimestamp = 0;
}

bool VROObjectRecognizeriOS::initObjectTracking(VROCameraPosition position,
                                                std::shared_ptr<VRODriver> driver) {
    
    
    _model = [[[model_pipelined alloc] init] model];
    _coreMLModel =  [VNCoreMLModel modelForMLModel:_model error:nil];
    _visionRequest = [[VNCoreMLRequest alloc] initWithModel:_coreMLModel
                                          completionHandler:(VNRequestCompletionHandler) ^(VNRequest *request, NSError *error) {
                                              NSArray *array = [request results];
                                              NSLog(@"Number of results %d", (int) array.count);
                                              
                                              std::map<std::string, std::vector<VRORecognizedObject>> objects;
                                              
                                              for (VNRecognizedObjectObservation *observation in array) {
                                                  CGRect bounds = observation.boundingBox;
                                                  NSLog(@"   Bounds x %f, y %f, w %f, h %f", bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
                                                  
                                                  VROBoundingBox box(bounds.origin.x,
                                                                     bounds.origin.x + bounds.size.width,
                                                                     1.0 - bounds.origin.y,
                                                                     1.0 - bounds.origin.y - bounds.size.height,
                                                                     0, 0);
                                                  for (VNClassificationObservation *classification in observation.labels) {
                                                      if (classification.confidence > 0.8) {
                                                          std::string className = std::string([classification.identifier UTF8String]);
                                                          
                                                          NSLog(@"   Label %@ confidence %f", classification.identifier, classification.confidence);
                                                          VROVector3f imagePoint(bounds.origin.x, bounds.origin.y, 0);
                                                          VROVector3f viewportPoint = _transform.multiply(imagePoint);
                                                          
                                                          NSLog(@"   Viewport point %f, %f", viewportPoint.x, viewportPoint.y);
                                                          objects[className].push_back({ className, box, classification.confidence });
                                                      }
                                                  }
                                              }
                                              
                                              dispatch_async(dispatch_get_main_queue(), ^{
                                                  std::shared_ptr<VROObjectRecognizerDelegate> delegate = _objectRecognizerDelegate_w.lock();
                                                  if (delegate) {
                                                      delegate->onObjectsFound(objects);
                                                  }
                                              });
                                          }];
    
    _visionRequest.preferBackgroundProcessing = YES;
    _visionRequest.imageCropAndScaleOption = VNImageCropAndScaleOptionScaleFill;
    
    return true;
}

void VROObjectRecognizeriOS::startObjectTracking() {
    
}

void VROObjectRecognizeriOS::stopObjectTracking() {
    
}

void VROObjectRecognizeriOS::trackWithVision(CVPixelBufferRef cameraImage, VROMatrix4f transform, VROCameraOrientation orientation) {
    double timestamp = VROTimeCurrentMillis();
    VROVector3f scale = transform.extractScale();
    VROVector3f translation = transform.extractTranslation();
    
    if ((timestamp - _lastTimestamp) > _fps) {
        _lastTimestamp = timestamp;
        CVPixelBufferRef convertedImage = VROImagePreprocessor::convertYCbCrToRGB(cameraImage);
        
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
            NSDictionary *visionOptions = [NSDictionary dictionary];
            
            size_t imageWidth, imageHeight;
            
            // The logic below accomplishes two things: it rotates the image right-side-up so that
            // it can be used by the ML algorithm, and it derives the _transform matrix, which is used
            // to convert *rotated* image coordinates to viewport coordinates. This matrix is derived
            // from the scale and translation components of ARKit's displayTransform metrix (we remove
            // the rotation part from the ARKit matrix because we're physically rotating the image
            // ourselves here).
            if (orientation == VROCameraOrientation::Portrait || orientation == VROCameraOrientation::PortraitUpsideDown) {
                // Remove rotation from the transformation matrix. Since this was a 90 degree rotation, X and Y are
                // reversed.
                _transform[0] = (int) scale.y;
                _transform[1] = 0;
                _transform[4] = 0;
                _transform[5] = scale.x;
                _transform[12] = (1 - scale.y) / 2.0;
                _transform[13] = translation.y;
                
                // The '3' here indicates 270 degree rotation
                CVPixelBufferRef rotatedImage = VROImagePreprocessor::rotateImage(convertedImage, 3, &imageWidth, &imageHeight);
                
                // Uncomment this to try the Boba/Axel static picture
                // UIImage *boba = [UIImage imageNamed:@"axel"];
                // VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCGImage:[boba CGImage] options:visionOptions];

                VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCVPixelBuffer:rotatedImage options:visionOptions];
                [handler performRequests:@[_visionRequest] error:nil];
                CVPixelBufferRelease(rotatedImage);
            }
            else if (orientation == VROCameraOrientation::LandscapeLeft) {
                // Remove rotation from the transformation matrix
                _transform[0] = scale.x;
                _transform[1] = 0;
                _transform[4] = 0;
                _transform[5] = scale.y;
                _transform[12] = (1 - scale.x) / 2.0;
                _transform[13] = (1 - scale.y) / 2.0;
                
                // The '2' here indicates 180 degree rotation
                CVPixelBufferRef rotatedImage = VROImagePreprocessor::rotateImage(convertedImage, 2, &imageWidth, &imageHeight);
                
                VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCVPixelBuffer:rotatedImage options:visionOptions];
                [handler performRequests:@[_visionRequest] error:nil];
                CVPixelBufferRelease(rotatedImage);
            }
            else if (orientation == VROCameraOrientation::LandscapeRight) {
                // In landscape right, the camera image is already right-side up, and ready for the ML
                // algorithm.
                _transform = transform;
                VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCVPixelBuffer:convertedImage options:visionOptions];
                [handler performRequests:@[_visionRequest] error:nil];
            }
            
            CVPixelBufferRelease(convertedImage);
        });
    }
}


