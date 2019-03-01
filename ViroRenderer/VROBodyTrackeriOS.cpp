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
#include <mutex>

#include "VROPoseFilterMovingAverage.h"
#include "VROPoseFilterLowPass.h"
#include "VROPoseFilterBoneDistance.h"
#include "VROPoseFilterEuro.h"

#define HOURGLASS_2_1 1
#define HOURGLASS_2_1_T_DS 1
#define HOURGLASS_2_1_T 1
#define HOURGLASS_4_1_T 2

// Set to one of the above
#define VRO_BODY_TRACKER_MODEL HOURGLASS_2_1

#if VRO_BODY_TRACKER_MODEL==HOURGLASS_2_1
#import "hourglass_2_1.h"
#elif VRO_BODY_TRACKER_MODEL==HOURGLASS_2_1_T_DS
#import "hourglass_2_1_t.h"
#elif VRO_BODY_TRACKER_MODEL==HOURGLASS_4_1_T
#import "hourglass_4_1_t.h"
#endif

#define VRO_PROFILE_NEURAL_ENGINE 0

static const float kConfidenceThreshold = 0.15;
static const float kInitialDampeningPeriodMs = 125;

static const bool kBodyTrackerDiscardPelvisAndThorax = false;

std::map<int, VROBodyJointType> _mpiiTypesToJointTypes = {
    { 0, VROBodyJointType::RightAnkle },
    { 1, VROBodyJointType::RightKnee },
    { 2, VROBodyJointType::RightHip },
    { 3, VROBodyJointType::LeftHip },
    { 4, VROBodyJointType::LeftKnee },
    { 5, VROBodyJointType::LeftAnkle },
    { 6, VROBodyJointType::Pelvis },
    { 7, VROBodyJointType::Thorax },
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
    { 14, VROBodyJointType::Thorax },
    { 15, VROBodyJointType::Pelvis },
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
    _visionQueue = dispatch_queue_create("com.viro.bodyTrackerVisionQueue", DISPATCH_QUEUE_SERIAL);
    _isTracking = false;
    _fpsTickIndex = 0;
    _fpsTickSum = 0;
    _neuralEngineInitialized = false;
    _isProcessingImage = false;
    _nextImage = nil;
    memset(_fpsTickArray, 0x0, sizeof(_fpsTickArray));
}

VROBodyTrackeriOS::~VROBodyTrackeriOS() {
    
}

bool VROBodyTrackeriOS::initBodyTracking(VROCameraPosition position,
                                         std::shared_ptr<VRODriver> driver) {

#if VRO_BODY_TRACKER_MODEL==HOURGLASS_2_1
    pinfo("Loading HG_2-1 body tracking model");
    _model = [[[hourglass_2_1 alloc] init] model];
#elif VRO_BODY_TRACKER_MODEL==HOURGLASS_2_1_T
    pinfo("Loading HG_2-1-T body tracking model");
    _model = [[[hourglass_2_1_t alloc] init] model];
#endif
    
    _cropAndScaleOption = VNImageCropAndScaleOptionCenterCrop;
    _coreMLModel =  [VNCoreMLModel modelForMLModel:_model error:nil];
    _visionRequest = [[VNCoreMLRequest alloc] initWithModel:_coreMLModel
                                          completionHandler:(VNRequestCompletionHandler)^(VNRequest *request, NSError *error) {
                                              processVisionResults(request, error);
                                          }];
    _visionRequest.imageCropAndScaleOption = _cropAndScaleOption;
    _poseFilter = std::make_shared<VROPoseFilterEuro>(kInitialDampeningPeriodMs, kConfidenceThreshold);

    return true;
}

void VROBodyTrackeriOS::setDampeningPeriodMs(double period) {
    _dampeningPeriodMs = period;
    if (period <= 0) {
        _poseFilter = nullptr;
    } else {
        _poseFilter = std::make_shared<VROPoseFilterEuro>(_dampeningPeriodMs, kConfidenceThreshold);
    }
}

double VROBodyTrackeriOS::getDampeningPeriodMs() const {
    return _dampeningPeriodMs;
}

VROPoseFrame VROBodyTrackeriOS::convertHeatmap(MLMultiArray *heatmap, VROMatrix4f transform) {
    if (heatmap.shape.count < 3) {
        return {};
    }
        
    int numJoints = (int) heatmap.shape[0].integerValue;
    int heatmapHeight = (int) heatmap.shape[1].integerValue;
    int heatmapWidth = (int) heatmap.shape[2].integerValue;
    
    passert (heatmap.dataType == MLMultiArrayDataTypeFloat32);
    float *array = (float *) heatmap.dataPointer;
    int stride_c = (int) heatmap.strides[0].integerValue;
    int stride_h = (int) heatmap.strides[1].integerValue;
    
    VROInferredBodyJoint bodyMap[kNumBodyJoints];
    double creationTime = VROTimeCurrentMillis();
    
    /*
     The ML model will return the heatmap tiles for each joint; choose the highest
     confidence tile for each joint.
     */
    for (int k = 0; k < numJoints; k++) {
#if VRO_BODY_TRACKER_MODEL==CPM
        VROBodyJointType type = (VROBodyJointType) k;
#elif VRO_BODY_TRACKER_MODEL==HOURGLASS_PIL
        VROBodyJointType type = _pilTypesToJointTypes[k];
#else
        VROBodyJointType type = _mpiiTypesToJointTypes[k];
#endif
        if (kBodyTrackerDiscardPelvisAndThorax &&
            (type == VROBodyJointType::Thorax || type == VROBodyJointType::Pelvis)) {
            continue;
        }
        if (type == VROBodyJointType::Unknown) {
            continue;
        }
        
        for (int i = 0; i < heatmapHeight; i++) {
            for (int j = 0; j < heatmapWidth; j++) {
                long index = k * stride_c + i * stride_h + j;
                float confidence = array[index];
                
                if (confidence > 0) {
                    VROInferredBodyJoint &joint = bodyMap[(int) type];
                    
                    /*
                     The point we create here is just the index of the heatmap tile
                     (i and j). We will convert this into a floating point value once
                     we find the highest confidence tile.
                     */
                    if (confidence > joint.getConfidence()) {
                        VROInferredBodyJoint inferredJoint(type);
                        inferredJoint.setConfidence(confidence);
                        inferredJoint.setTileIndices(j, i);
                        inferredJoint.setCreationTime(creationTime);
                        
                        bodyMap[(int) type] = inferredJoint;
                    }
                }
            }
        }
    }
    
    /*
     Now we have a map with the highest confidence tile for each joint. Convert the
     heatmap tile indices into normalized coordinates [0, 1].
     */
    for (VROInferredBodyJoint &joint : bodyMap) {
        if (joint.getConfidence() > 0) {
            VROVector3f    tilePoint  = { (float) joint.getTileX(), (float) joint.getTileY() };
            
            // Convert tile indices to normalized camera image coordinates [0, 1]
            VROVector3f imagePoint = { (tilePoint.x + 0.5f) / (float) (heatmapWidth),
                                       (tilePoint.y + 0.5f) / (float) (heatmapHeight), 0 };
            
            // Multiply by the ARKit transform to get normalized viewport coordinates [0, 1]
            VROVector3f viewportPoint = transform.multiply(imagePoint);
            VROBoundingBox viewportBounds = VROBoundingBox(viewportPoint.x, viewportPoint.x, viewportPoint.y, viewportPoint.y, 0, 0);
            joint.setBounds(viewportBounds);
        }
    }
    
    VROPoseFrame poseFrame = newPoseFrame();
    for (int i = 0; i < kNumBodyJoints; i++) {
        VROInferredBodyJoint &inferredJoint = bodyMap[i];
        if (inferredJoint.getConfidence() > 0) {
            poseFrame[i].push_back(inferredJoint);
        }
    }
    return poseFrame;
}

void VROBodyTrackeriOS::startBodyTracking() {
    _isTracking = true;
}

void VROBodyTrackeriOS::stopBodyTracking() {
    _isTracking = false;
}

void VROBodyTrackeriOS::update(const VROARFrame &frame) {
    if (!_isTracking) {
        return;
    }
    
    const VROARFrameiOS &frameiOS = (VROARFrameiOS &)frame;
    
    // Store the given image, which we'll run when the neural engine
    // is done processing its current image
    
    {
        std::lock_guard<std::mutex> lock(_imageMutex);
        if (_nextImage) {
            CVBufferRelease(_nextImage);
        }
        _nextImage = CVBufferRetain(frameiOS.getImage());
        _nextTransform = frameiOS.getCameraImageToViewportTransform();
        _nextOrientation = frameiOS.getCameraOrientation();
    }

    // Start tracking images if we haven't yet initialized
    if (!_neuralEngineInitialized) {
        _neuralEngineInitialized = true;
        _nanosecondsLastFrame = VRONanoTime();
    }
    
    // Try to process the next image (this will no-op if the neural
    // engine is already processing an image)
    std::weak_ptr<VROBodyTrackeriOS> tracker_w = std::dynamic_pointer_cast<VROBodyTrackeriOS>(shared_from_this());
    dispatch_async(_visionQueue, ^{
        std::shared_ptr<VROBodyTrackeriOS> tracker = tracker_w.lock();
        if (tracker) {
            tracker->nextImage();
        }
    });
}

// Invoked on the _visionQueue
void VROBodyTrackeriOS::nextImage() {
#if VRO_PROFILE_NEURAL_ENGINE
    NSLog(@"Starting neural engine frame");
#endif
    
    CVPixelBufferRef image = nullptr;
    {
        std::lock_guard<std::mutex> lock(_imageMutex);
        // Only process one image at a time
        if (_isProcessingImage || !_nextImage) {
            return;
        }
        _isProcessingImage = true;
        image = CVBufferRetain(_nextImage);
    }
    
#if VRO_PROFILE_NEURAL_ENGINE
    NSLog(@"   Idle time %f", VROTimeCurrentMillis() - _betweenImageTime);
#endif

    trackImage(image, _nextTransform, _nextOrientation);
    CVBufferRelease(image);

    {
        std::lock_guard<std::mutex> lock(_imageMutex);
        _isProcessingImage = false;
    }
    _betweenImageTime = VROTimeCurrentMillis();
    
    // Use dispatch_async to prevent recursion overflow
    std::weak_ptr<VROBodyTrackeriOS> tracker_w = std::dynamic_pointer_cast<VROBodyTrackeriOS>(shared_from_this());
    dispatch_async(_visionQueue, ^{
        std::shared_ptr<VROBodyTrackeriOS> tracker = tracker_w.lock();
        if (tracker) {
            tracker->nextImage();
        }
    });
}

// Invoked on the _visionQueue
void VROBodyTrackeriOS::trackImage(CVPixelBufferRef image, VROMatrix4f transform, VROCameraOrientation orientation) {
    NSDictionary *visionOptions = [NSDictionary dictionary];
    
    // The logic below derives the _transform matrix, which is used to convert *rotated* image
    // coordinates to viewport coordinates. This matrix is derived from the scale and translation
    // components of ARKit's displayTransform metrix (we remove the rotation part from the ARKit
    // matrix because iOS will automatically rotate the image before inputting it into the CoreML
    // model).
    VROVector3f scale = transform.extractScale();
    VROVector3f translation = transform.extractTranslation();
    
    if (orientation == VROCameraOrientation::Portrait || orientation == VROCameraOrientation::PortraitUpsideDown) {
        float width = (float) CVPixelBufferGetWidth(image);
        float height = (float) CVPixelBufferGetHeight(image);
        
        // Remove rotation from the transformation matrix. Since this was a 90 degree rotation, X and Y are
        // reversed.
        //
        // Note also that the transform used depends on the VNImageCropAndScale option used. CenterCrop
        // is preferred because it highlights the body in the center of the image, and is documented to
        // preserve aspect ratio (Apple documentation is unclear about aspect ratio preservation for the
        // other modes).
        if (_cropAndScaleOption == VNImageCropAndScaleOptionCenterCrop) {
            _transform[0] = scale.y;
            _transform[1] = 0;
            _transform[4] = 0;
            _transform[5] = scale.x * height / width;
            _transform[12] = (1 - scale.y) / 2.0;
            _transform[13] = ((width - height) / width) / 2.0;
        }
        else { // ScaleToFill: note, this appears to not preserve aspect ratio (not confirmed)
            _transform[0] = scale.y;
            _transform[1] = 0;
            _transform[4] = 0;
            _transform[5] = scale.x;
            _transform[12] = (1 - scale.y) / 2.0;
            _transform[13] = translation.y;
        }
        
        // iOS always rotates the image right-side up before inputting into a CoreML model for
        // a vision request. We ensure it rotates correctly by specifying the orientation of the
        // image with respect to the device.
        CGImagePropertyOrientation orientation = kCGImagePropertyOrientationRight;
        
        // By wrapping the CVPixelBuffer in a CIImage, iOS will automatically convert from
        // YCbCr to RGB (note: this is undocumented, but works).
        _startNeural = VROTimeCurrentMillis();
        CIImage *ciImage = [[CIImage alloc] initWithCVPixelBuffer:image];

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
        CIImage *ciImage = [[CIImage alloc] initWithCVPixelBuffer:image];
        VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCIImage:ciImage
                                                                            orientation:orientation
                                                                                options:visionOptions];
        [handler performRequests:@[_visionRequest] error:nil];
    }
    else if (orientation == VROCameraOrientation::LandscapeRight) {
        // In landscape right, the camera image is already right-side up, and ready for the ML
        // algorithm.
        _transform = transform;
        
        CIImage *ciImage = [[CIImage alloc] initWithCVPixelBuffer:image];
        VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCIImage:ciImage options:visionOptions];
        [handler performRequests:@[_visionRequest] error:nil];
    }
}

// Invoked on the _visionQueue
void VROBodyTrackeriOS::processVisionResults(VNRequest *request, NSError *error) {
#if VRO_PROFILE_NEURAL_ENGINE
    NSLog(@"   Neural engine time %f", VROTimeCurrentMillis() - _startNeural);
#endif
    _startHeatmap = VROTimeCurrentMillis();
    NSArray *array = [request results];
    
    VNCoreMLFeatureValueObservation *topResult = (VNCoreMLFeatureValueObservation *)(array[0]);
    MLMultiArray *heatmap = topResult.featureValue.multiArrayValue;
    VROPoseFrame joints = convertHeatmap(heatmap, _transform);
    
#if VRO_PROFILE_NEURAL_ENGINE
    NSLog(@"   Heatmap processing time %f", VROTimeCurrentMillis() - _startHeatmap);
#endif

    std::weak_ptr<VROBodyTrackeriOS> tracker_w = std::dynamic_pointer_cast<VROBodyTrackeriOS>(shared_from_this());

    dispatch_async(dispatch_get_main_queue(), ^{
        std::shared_ptr<VROBodyTrackeriOS> tracker = tracker_w.lock();
        
        if (tracker && tracker->_isTracking) {
            std::shared_ptr<VROBodyTrackerDelegate> delegate = _bodyMeshDelegate_w.lock();
            
            if (delegate) {
                VROPoseFrame dampenedJoints = newPoseFrame();
                if (!_poseFilter) {
                    dampenedJoints = joints;
                } else {
                    dampenedJoints = tracker->_poseFilter->filterJoints(joints);
                }
                delegate->onBodyJointsFound(dampenedJoints);
            }
        }
    });
    
    // Compute FPS
    uint64_t nanosecondsThisFrame = VRONanoTime();
    uint64_t tick = nanosecondsThisFrame - _nanosecondsLastFrame;
    _nanosecondsLastFrame = nanosecondsThisFrame;
    updateFPS(tick);
    
#if VRO_PROFILE_NEURAL_ENGINE
    NSLog(@"Neural Engine FPS %f\n", getFPS());
#endif
}

void VROBodyTrackeriOS::updateFPS(uint64_t newTick) {
    // Simple moving average: subtract value falling off, and add new value
    
    _fpsTickSum -= _fpsTickArray[_fpsTickIndex];
    _fpsTickSum += newTick;
    _fpsTickArray[_fpsTickIndex] = newTick;
    
    if (++_fpsTickIndex == kNeuralFPSMaxSamples) {
        _fpsTickIndex = 0;
    }
}

double VROBodyTrackeriOS::getFPS() const {
    double averageNanos = ((double) _fpsTickSum) / kNeuralFPSMaxSamples;
    return 1.0 / (averageNanos / (double) 1e9);
}
