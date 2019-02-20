//
//  VROBodyTrackeriOS.cpp
//  ViroKit
//
//  Created by vik.advani on 9/4/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROBodyTrackeriOS.h"
#include "VROLog.h"
#include "VROTime.h"
#include <Accelerate/Accelerate.h>
#include "VROImagePreprocessor.h"
#include "VRODriverOpenGLiOS.h"
#include "VROARFrameiOS.h"

#define CPM 0
#define HOURGLASS_2_1_T 1
#define HOURGLASS_4_1_T 2
#define HOURGLASS_4_2 3
#define HOURGLASS_8_1 4

// Set to CPM, HOURGLASS_2_1_T, HOURGLASS_4_1_T, HOURGLASS_4_2, HOURGLASS_8_1
#define VRO_BODY_TRACKER_MODEL HOURGLASS_2_1_T

#if VRO_BODY_TRACKER_MODEL==CPM
#import "model_cpm.h"
#elif VRO_BODY_TRACKER_MODEL==HOURGLASS_2_1_T
#import "hourglass_2_1_t.h"
#elif VRO_BODY_TRACKER_MODEL==HOURGLASS_4_1_T
#import "hourglass_4_1_t.h"
#elif VRO_BODY_TRACKER_MODEL==HOURGLASS_4_2
#import "hourglass_4_2.h"
#elif VRO_BODY_TRACKER_MODEL==HOURGLASS_8_1
#import "model_hourglass.h"
#else
#endif

std::map<int, VROBodyJointType> _mpiiTypesToJointTypes = {
    { 0, VROBodyJointType::RightAnkle },
    { 1, VROBodyJointType::RightKnee },
    { 2, VROBodyJointType::RightHip },
    { 3, VROBodyJointType::LeftHip },
    { 4, VROBodyJointType::LeftKnee },
    { 5, VROBodyJointType::LeftAnkle },
    { 6, VROBodyJointType::Unknown },
    { 7, VROBodyJointType::Unknown },
    { 8, VROBodyJointType::Neck },
    { 9, VROBodyJointType::Top },
    { 10, VROBodyJointType::RightWrist },
    { 11, VROBodyJointType::RightElbow },
    { 12, VROBodyJointType::RightShoulder },
    { 13, VROBodyJointType::LeftShoulder },
    { 14, VROBodyJointType::LeftElbow },
    { 15, VROBodyJointType::LeftWrist },
};

std::map<int, VROBodyJointType> _pilTypesToJointTypes = {
    { 0, VROBodyJointType::Top },
    { 1, VROBodyJointType::Neck },
    { 2, VROBodyJointType::RightShoulder },
    { 3, VROBodyJointType::RightElbow },
    { 4, VROBodyJointType::RightWrist },
    { 5, VROBodyJointType::LeftShoulder },
    { 6, VROBodyJointType::LeftElbow },
    { 7, VROBodyJointType::LeftWrist },
    { 8, VROBodyJointType::RightHip },
    { 9, VROBodyJointType::RightKnee },
    { 10, VROBodyJointType::RightAnkle },
    { 11, VROBodyJointType::LeftHip },
    { 12, VROBodyJointType::LeftKnee },
    { 13, VROBodyJointType::LeftAnkle },
    { 14, VROBodyJointType::Unknown },
    { 15, VROBodyJointType::Unknown },
};

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

VROBodyTrackeriOS::VROBodyTrackeriOS() {
    _currentImage = nil;
    _visionQueue = dispatch_queue_create("com.viro.bodyTrackerYoloVisionQueue", DISPATCH_QUEUE_SERIAL);
    _isTracking = false;
}

bool VROBodyTrackeriOS::initBodyTracking(VROCameraPosition position,
                                         std::shared_ptr<VRODriver> driver) {
    
#if VRO_BODY_TRACKER_MODEL==CPM
    pinfo("Loading CPM body tracking model");
    _model = [[[model_cpm alloc] init] model];
#elif VRO_BODY_TRACKER_MODEL==HOURGLASS_2_1_T
    pinfo("Loading HG_2-1-T body tracking model");
    _model = [[[hourglass_2_1_t alloc] init] model];
#elif VRO_BODY_TRACKER_MODEL==HOURGLASS_4_1_T
    pinfo("Loading HG_4-1-t body tracking model");
    _model = [[[hourglass_4_1_t alloc] init] model];
#elif VRO_BODY_TRACKER_MODEL==HOURGLASS_4_2
    pinfo("Loading HG-4-2 body tracking model");
    _model = [[[hourglass_4_2 alloc] init] model];
#elif VRO_BODY_TRACKER_MODEL==HOURGLASS_8_1
    pinfo("Loading HG-8-1 body tracking model");
    _model = [[[model_hourglass alloc] init] model];
#endif
    
    _coreMLModel =  [VNCoreMLModel modelForMLModel:_model error:nil];
    _visionRequest = [[VNCoreMLRequest alloc] initWithModel:_coreMLModel
                                          completionHandler:(VNRequestCompletionHandler)^(VNRequest *request, NSError *error) {
                                              processVisionResults(request, error);
                                          }];
    _visionRequest.imageCropAndScaleOption = VNImageCropAndScaleOptionScaleFill;
    return true;
}

std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> VROBodyTrackeriOS::convertHeatmap(MLMultiArray *heatmap, VROMatrix4f transform) {
    if (heatmap.shape.count < 3) {
        return {};
    }
        
    int keypoint = (int) heatmap.shape[0].integerValue;
    int heatmapWidth = (int) heatmap.shape[1].integerValue;
    int heatmapHeight = (int) heatmap.shape[2].integerValue;
    
    std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> bodyMap;
    
    /*
     The ML model will return the heatmap tiles for each joint; choose the highest
     confidence tile for each joint.
     */
    for (int k = 0; k < keypoint; k++) {
#if VRO_BODY_TRACKER_MODEL==CPM
        VROBodyJointType type = (VROBodyJointType) k;
#elif VRO_BODY_TRACKER_MODEL==HOURGLASS_8_1
        VROBodyJointType type = _mpiiTypesToJointTypes[k];
#else
        VROBodyJointType type = _pilTypesToJointTypes[k];
#endif
        if (type == VROBodyJointType::Unknown) {
            continue;
        }
        
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
                    if (kv == bodyMap.end() || kv->second.empty() || confidence > kv->second[0].getConfidence()) {
                        VROVector3f point(CGFloat(j), CGFloat(i), 0);
                        bodyMap[type].clear();
                        
                        VROBoundingBox bounds = VROBoundingBox(point.x, point.x, point.y, point.y, 0, 0);
                        VROInferredBodyJoint inferredJoint = { type, bounds, confidence };
                        bodyMap[type].push_back(inferredJoint);
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
        for (VROInferredBodyJoint &joint : kv.second) {
            VROBoundingBox tileBounds = joint.getBounds();
            VROVector3f tilePoint = { tileBounds.getX(), tileBounds.getY() };
            
            // Convert tile indices to normalized camera image coordinates [0, 1]
            VROVector3f imagePoint = { (tilePoint.x + 0.5f) / (float) (heatmapWidth),
                (tilePoint.y + 0.5f) / (float) (heatmapHeight), 0 };
            
            // Multiply by the ARKit transform to get normalized viewport coordinates [0, 1]
            VROVector3f viewportPoint = transform.multiply(imagePoint);
            VROBoundingBox viewportBounds = VROBoundingBox(viewportPoint.x, viewportPoint.x, viewportPoint.y, viewportPoint.y, 0, 0);
            joint.setBounds(viewportBounds);
        }
    }
    return bodyMap;
}

void VROBodyTrackeriOS::startBodyTracking() {
    _isTracking = true;
}

void VROBodyTrackeriOS::stopBodyTracking() {
    _isTracking = false;
}

void VROBodyTrackeriOS::update(const VROARFrame &frame) {
    const VROARFrameiOS &frameiOS = (VROARFrameiOS &)frame;
    
    CVPixelBufferRef cameraImage = frameiOS.getImage();
    VROMatrix4f transform = frameiOS.getCameraImageToViewportTransform();
    VROCameraOrientation orientation = frameiOS.getCameraOrientation();
    
    // Only process one image at a time
    if (_currentImage != nil) {
        return;
    }
    _currentImage = cameraImage;
    
    dispatch_async(_visionQueue, ^{
        trackCurrentImage(transform, orientation);
    });
}

// Invoked on the _visionQueue
void VROBodyTrackeriOS::trackCurrentImage(VROMatrix4f transform, VROCameraOrientation orientation) {
    NSDictionary *visionOptions = [NSDictionary dictionary];
    
    // The logic below derives the _transform matrix, which is used to convert *rotated* image
    // coordinates to viewport coordinates. This matrix is derived from the scale and translation
    // components of ARKit's displayTransform metrix (we remove the rotation part from the ARKit
    // matrix because iOS will automatically rotate the image before inputting it into the CoreML
    // model).
    VROVector3f scale = transform.extractScale();
    VROVector3f translation = transform.extractTranslation();
    
    if (orientation == VROCameraOrientation::Portrait || orientation == VROCameraOrientation::PortraitUpsideDown) {
        // Remove rotation from the transformation matrix. Since this was a 90 degree rotation, X and Y are
        // reversed.
        _transform[0] = scale.y;
        _transform[1] = 0;
        _transform[4] = 0;
        _transform[5] = scale.x;
        _transform[12] = (1 - scale.y) / 2.0;
        _transform[13] = translation.y;
        
        // iOS always rotates the image right-side up before inputting into a CoreML model for
        // a vision request. We ensure it rotates correctly by specifying the orientation of the
        // image with respect to the device.
        CGImagePropertyOrientation orientation = kCGImagePropertyOrientationRight;
        
        // By wrapping the CVPixelBuffer in a CIImage, iOS will automatically convert from
        // YCbCr to RGB (note: this is undocumented, but works).
        CIImage *ciImage = [[CIImage alloc] initWithCVPixelBuffer:_currentImage];
        VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCIImage:ciImage
                                                                            orientation:orientation
                                                                                options:visionOptions];
        [handler performRequests:@[_visionRequest] error:nil];
    }
    else if (orientation == VROCameraOrientation::LandscapeLeft) {
        // Remove rotation from the transformation matrix
        _transform[0] = scale.x;
        _transform[1] = 0;
        _transform[4] = 0;
        _transform[5] = scale.y;
        _transform[12] = (1 - scale.x) / 2.0;
        _transform[13] = (1 - scale.y) / 2.0;
        
        CGImagePropertyOrientation orientation = kCGImagePropertyOrientationDown;
        CIImage *ciImage = [[CIImage alloc] initWithCVPixelBuffer:_currentImage];
        VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCIImage:ciImage
                                                                            orientation:orientation
                                                                                options:visionOptions];
        [handler performRequests:@[_visionRequest] error:nil];
    }
    else if (orientation == VROCameraOrientation::LandscapeRight) {
        // In landscape right, the camera image is already right-side up, and ready for the ML
        // algorithm.
        _transform = transform;
        
        CIImage *ciImage = [[CIImage alloc] initWithCVPixelBuffer:_currentImage];
        VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCIImage:ciImage options:visionOptions];
        [handler performRequests:@[_visionRequest] error:nil];
    }
}

// Invoked on the _visionQueue
void VROBodyTrackeriOS::processVisionResults(VNRequest *request, NSError *error) {
    NSArray *array = [request results];
    
    VNCoreMLFeatureValueObservation *topResult = (VNCoreMLFeatureValueObservation *)(array[0]);
    MLMultiArray *heatmap = topResult.featureValue.multiArrayValue;
    std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> joints = convertHeatmap(heatmap, _transform);
    
    dispatch_async(dispatch_get_main_queue(), ^{
        std::shared_ptr<VROBodyTrackerDelegate> delegate = _bodyMeshDelegate_w.lock();
        if (delegate && _isTracking) {
            delegate->onBodyJointsFound(joints);
        }
    });
    
    _currentImage = nil;
}
