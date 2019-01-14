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

static int kImageWriteFrame = 0;

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
                                              
                                              std::map<std::string, VRORecognizedObject> objects;
                                              
                                              for (VNRecognizedObjectObservation *observation in array) {
                                                  CGRect bounds = observation.boundingBox;
                                                  NSLog(@"Bounds x %f, y %f, w %f, h %f", bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
                                                  for (VNClassificationObservation *classification in observation.labels) {
                                                      if (classification.confidence > 0.8) {
                                                          std::string className = std::string([classification.identifier UTF8String]);
                                                          
                                                          NSLog(@"   Label %@ confidence %f", classification.identifier, classification.confidence);
                                                          VROVector3f imagePoint(bounds.origin.x, bounds.origin.y, 0);
                                                          VROVector3f viewportPoint = _transform.multiply(imagePoint);
                                                          objects[className] = { className, viewportPoint, classification.confidence };
                                                      }
                                                  }
                                              }
                                              
                                              dispatch_async(dispatch_get_main_queue(), ^{
                                                  std::shared_ptr<VROObjectRecognizerDelegate> delegate = _objectRecognizerDelegate_w.lock();
                                                  if (delegate) {
                                                      delegate->onObjectsFound(objects);
                                                  }
                                              });
                                              /*
                                              VNCoreMLFeatureValueObservation *topResult = (VNCoreMLFeatureValueObservation *)(array[0]);
                                              MLMultiArray *heatmap = topResult.featureValue.multiArrayValue;
                                              std::map<VRORecognizedObjectType, VRORecognizedObject> joints = convertHeatmap(heatmap, _transform);
                                              
                                              
                                               */
                                          }];
    
    _visionRequest.preferBackgroundProcessing = YES;
    _visionRequest.imageCropAndScaleOption = VNImageCropAndScaleOptionScaleFill;
    
    return true;
}

std::map<std::string, VRORecognizedObject> VROObjectRecognizeriOS::convertHeatmap(MLMultiArray *heatmap, VROMatrix4f transform) {
    if (heatmap.shape.count < 3) {
        return {};
    }
    
    int keypoint = (int) heatmap.shape[0].integerValue;
    int heatmapWidth = (int) heatmap.shape[1].integerValue;
    int heatmapHeight = (int) heatmap.shape[2].integerValue;
    
    std::map<std::string, VRORecognizedObject> bodyMap;
    
    /*
     The ML model will return the heatmap tiles for each joint; choose the highest
     confidence tile for each joint.
     */
    for (int k = 0; k < keypoint; k++) {
        std::string type = "";
        
        for (int i = 0; i < heatmapWidth; i++) {
            for (int j = 0; j < heatmapHeight; j++) {
                long index = k * (heatmapWidth * heatmapHeight) + i * (heatmapHeight) + j;
                double confidence = heatmap[index].doubleValue;
                
                if (confidence > 0) {
                    auto kv = bodyMap.find(type);
                    
                    /*
                     The point we create here is just the index of the heatmap tile
                     (i and j). We will convert this into a floating point value once
                     we find the highest confidence tile.
                     */
                    if (kv == bodyMap.end() || confidence > kv->second.getConfidence()) {
                        VROVector3f point(CGFloat(j), CGFloat(i), 0);
                        bodyMap[type] = { type, point, confidence };
                    }
                }
            }
        }
    }
    
    /*
     Now we have a map with the highest confidence tile for each joint. Convert the
     heatmap tile indices into normalized coordinates [0, 1].
     */
    for (auto &kv : bodyMap) {
        VRORecognizedObject &joint = kv.second;
        VROVector3f tilePoint = joint.getScreenCoords();
        
        // Convert tile indices to normalized camera image coordinates [0, 1]
        VROVector3f imagePoint = { (tilePoint.x + 0.5f) / (float) (heatmapWidth),
            (tilePoint.y + 0.5f) / (float) (heatmapHeight), 0 };
        
        // Multiply by the ARKit transform to get normalized viewport coordinates [0, 1]
        VROVector3f viewportPoint = transform.multiply(imagePoint);
       
    }
    return bodyMap;
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
            
            size_t imageWidth = CVPixelBufferGetWidth(convertedImage);
            size_t imageHeight = CVPixelBufferGetHeight(convertedImage);
            
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
                CVPixelBufferRef rotatedImage = VROImagePreprocessor::rotateImage(convertedImage, 3, imageHeight, imageWidth);
                
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
                CVPixelBufferRef rotatedImage = VROImagePreprocessor::rotateImage(convertedImage, 2, imageWidth, imageHeight);
                
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


