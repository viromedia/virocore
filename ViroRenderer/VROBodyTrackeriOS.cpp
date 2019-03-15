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
#include "VROARFrameInertial.h"
#include <mutex>
#include "VRODeviceUtil.h"

#include "VROPoseFilterMovingAverage.h"
#include "VROPoseFilterLowPass.h"
#include "VROPoseFilterBoneDistance.h"
#include "VROPoseFilterEuro.h"
#include "VROOneEuroFilter.h"

#define HOURGLASS_2_1 1
#define HOURGLASS_2_1_T_DS 2
#define HOURGLASS_2_1_T 3

// Set to one of the above
#define VRO_BODY_TRACKER_MODEL_A12 HOURGLASS_2_1_T
#define VRO_BODY_TRACKER_MODEL_A11 HOURGLASS_2_1_DS_T

#import "hourglass_2_1.h"
#import "hourglass_2_1_t.h"
#import "hourglass_2_1_ds_t.h"

#define VRO_PROFILE_NEURAL_ENGINE 0

static const float kConfidenceThreshold = 0.15;
static const float kInitialDampeningPeriodMs = 125;

// The size of the image input into the CoreML model
static const int kVisionImageSize = 256;

// Set to true to save the crop and padding result of one of the early
// frames to the Photo Library, for debubbing.
static bool kDebugCropAndPadResult = false;
static const int kDebugCropAndPadResultFrame = 30;

// Parameterization for the One Euro filter used when dynamic cropping.
static const double kCropEuroFilterFrequency = 60;
static const double kCropEuroFilterDCutoff = 1;
static const double kCropEuroFilterBeta = 1.0;
static const double kCropEuroFilterFCMin = 1.7;
static const double kCropPaddingMultiplier = 0.1;

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
    _cropScratchBuffer = nil;
    _cropScratchBufferLength = 0;
    
    _dynamicCropBox = CGRectNull;
    _dynamicCropXFilter = std::make_shared<VROOneEuroFilterF>(kCropEuroFilterFrequency, kCropEuroFilterFCMin,
                                                              kCropEuroFilterBeta, kCropEuroFilterDCutoff);
    _dynamicCropYFilter = std::make_shared<VROOneEuroFilterF>(kCropEuroFilterFrequency, kCropEuroFilterFCMin,
                                                              kCropEuroFilterBeta, kCropEuroFilterDCutoff);
    _dynamicCropWidthFilter = std::make_shared<VROOneEuroFilterF>(kCropEuroFilterFrequency, kCropEuroFilterFCMin,
                                                                  kCropEuroFilterBeta, kCropEuroFilterDCutoff);
    _dynamicCropHeightFilter = std::make_shared<VROOneEuroFilterF>(kCropEuroFilterFrequency, kCropEuroFilterFCMin,
                                                                   kCropEuroFilterBeta, kCropEuroFilterDCutoff);
    memset(_fpsTickArray, 0x0, sizeof(_fpsTickArray));
}

VROBodyTrackeriOS::~VROBodyTrackeriOS() {
    if (_cropScratchBuffer != nullptr) {
        free(_cropScratchBuffer);
    }
}

bool VROBodyTrackeriOS::initBodyTracking(VROCameraPosition position,
                                         std::shared_ptr<VRODriver> driver) {

    VRODeviceUtil *device = [[VRODeviceUtil alloc] init];
    bool A12 = [device isBionicA12];
    
    if (A12) {
#if VRO_BODY_TRACKER_MODEL_A12==HOURGLASS_2_1
        pinfo("Loading HG_2-1 body tracking model");
        _model = [[[hourglass_2_1 alloc] init] model];
#elif VRO_BODY_TRACKER_MODEL_A12==HOURGLASS_2_1_T
        pinfo("Loading HG_2-1-T body tracking model");
        _model = [[[hourglass_2_1_t alloc] init] model];
#endif
    } else {
#if VRO_BODY_TRACKER_MODEL_A11==HOURGLASS_2_1
        pinfo("Loading HG_2-1 body tracking model");
        _model = [[[hourglass_2_1 alloc] init] model];
#elif VRO_BODY_TRACKER_MODEL_A11==HOURGLASS_2_1_T
        pinfo("Loading HG_2-1-T body tracking model");
        _model = [[[hourglass_2_1_t alloc] init] model];
#elif VRO_BODY_TRACKER_MODEL_A11==HOURGLASS_2_1_DS_T
        pinfo("Loading HG_2-1-T-ds body tracking model");
        _model = [[[hourglass_2_1_ds_t alloc] init] model];
#endif
    }

    _cameraPosition = position;
    _cropAndScaleOption = VROCropAndScaleOption::CoreML_FitCrop;
    
    _coreMLModel =  [VNCoreMLModel modelForMLModel:_model error:nil];
    _visionRequest = [[VNCoreMLRequest alloc] initWithModel:_coreMLModel
                                          completionHandler:(VNRequestCompletionHandler)^(VNRequest *request, NSError *error) {
                                              processVisionResults(request, error);
                                          }];
    
    switch (_cropAndScaleOption) {
        case VROCropAndScaleOption::CoreML_Fill:
            _visionRequest.imageCropAndScaleOption = VNImageCropAndScaleOptionScaleFill;
            break;
        case VROCropAndScaleOption::CoreML_Fit:
            _visionRequest.imageCropAndScaleOption = VNImageCropAndScaleOptionScaleFit;
            break;
        case VROCropAndScaleOption::CoreML_FitCrop:
            _visionRequest.imageCropAndScaleOption = VNImageCropAndScaleOptionCenterCrop;
            break;
        default:
            // For Viro cropping, disable CoreML's cropping and scaling by setting it to ScaleFill
            _visionRequest.imageCropAndScaleOption = VNImageCropAndScaleOptionScaleFill;
            break;
    }
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

VROPoseFrame VROBodyTrackeriOS::convertHeatmap(MLMultiArray *heatmap, VROCameraPosition cameraPosition,
                                               VROMatrix4f visionToImageSpace, VROMatrix4f imageToViewportSpace,
                                               std::pair<VROVector3f, float> *outImageSpaceJoints) {
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
        VROBodyJointType type = _mpiiTypesToJointTypes[k];
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
            
            // Convert tile indices to vision coordinates [0, 1]
            VROVector3f visionPoint = { (tilePoint.x + 0.5f) / (float) (heatmapWidth),
                                        (tilePoint.y + 0.5f) / (float) (heatmapHeight), 0 };
            
            // Multiply by the ARKit transform to get normalized viewport coordinates [0, 1]
            VROVector3f imagePoint = visionToImageSpace.multiply(visionPoint);
            VROVector3f viewportPoint = imageToViewportSpace.multiply(imagePoint);
            
            // Store the image points for bounding box computation
            outImageSpaceJoints[(int) joint.getType()].first = imagePoint;
            outImageSpaceJoints[(int) joint.getType()].second = joint.getConfidence();
            
            // Mirror the X dimension if we're using the front-facing camera
            if (cameraPosition == VROCameraPosition::Front) {
                viewportPoint.x = 1.0 - viewportPoint.x;
            }
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

void VROBodyTrackeriOS::update(const VROARFrame *frame) {
    if (!_isTracking) {
        return;
    }
    
    const VROARFrameiOS *frameiOS = dynamic_cast<const VROARFrameiOS *>(frame);
    
    // Store the given image, which we'll run when the neural engine
    // is done processing its current image
    
    {
        std::lock_guard<std::mutex> lock(_imageMutex);
        _nextTransform = frame->getViewportToCameraImageTransform().invert();

        if (_nextImage) {
            CVBufferRelease(_nextImage);
        }
        if (frameiOS) {
            _nextImage = CVBufferRetain(frameiOS->getImage());
            _nextOrientation = frameiOS->getImageOrientation();
        } else {
            const VROARFrameInertial *frameInertial = dynamic_cast<const VROARFrameInertial *>(frame);
            CMSampleBufferRef sampleBuffer = frameInertial->getImage();
            _nextImage = CVBufferRetain(CMSampleBufferGetImageBuffer(sampleBuffer));
            _nextOrientation = frameInertial->getImageOrientation();
        }
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
void VROBodyTrackeriOS::trackImage(CVPixelBufferRef image, VROMatrix4f transform, CGImagePropertyOrientation orientation) {
    NSDictionary *visionOptions = [NSDictionary dictionary];
    
    // The logic below derives the _transform matrix, which is used to convert *rotated* image
    // coordinates to viewport coordinates. This matrix is derived from the scale and translation
    // components of ARKit's displayTransform matrix (we remove the rotation part from the ARKit
    // matrix because iOS will automatically rotate the image before inputting it into the CoreML
    // model).
    VROVector3f scale = transform.extractScale();
    VROVector3f translation = transform.extractTranslation();
    
    float width = (float) CVPixelBufferGetWidth(image);
    float height = (float) CVPixelBufferGetHeight(image);
    bool needsRelease = false;
    
    // Derive the toViewport matrix, which moves coordinates from image space
    // to viewport space.
    VROMatrix4f toViewport;
    if (orientation == kCGImagePropertyOrientationRight) {
        // For right orientation, our inverse viewport transformation is derived
        // from the ARKit transform. Because of the rotation, scale X and Y are
        // reversed. The translation in the X direction is (1 - scale.y) / 2.0,
        // but why is unclear.
        toViewport[0] = scale.y;
        toViewport[5] = scale.x;
        toViewport[12] = (1 - scale.y) / 2.0;
        toViewport[13] = 0;
        
        // Invert width and height for the invertViewport computations, because of
        // the rotation.
        float temp = width;
        width = height;
        height = temp;
    }
    else if (orientation == kCGImagePropertyOrientationUp) {
        // For up orientation, our invert viewport transformation is simply the inverse
        // of the display transformation, which we were given. There is no rotation
        // element to consider.
        toViewport[0] = scale.x;
        toViewport[5] = scale.y;
        toViewport[12] = translation.x;
        toViewport[13] = translation.y;
    }
    
    // Derive the toImage transformation, which moves from vision space (CoreML input
    // space) to image space.
    VROMatrix4f toImage;
    if (_cropAndScaleOption == VROCropAndScaleOption::CoreML_FitCrop) {
        // FitCrop works by fitting the short side (X), then cropping the long side (Y)
        // about the center, preserving aspect ratio. This means the long side (Y)
        // is scaled and translated. To undo this we invert both the scale and
        // translation.
        //
        // Note that we're working on normalized coordinates, so when computing this
        // transform we only consider the scale needed __in addition to__ the normal
        // scale that would come with scale to fill.
        //
        // For example, we have an image that's 1024x1920. To fit the short side, we shrink
        // that image to a square until the short side fits. This results in a long side
        // that extrudes outside of the square. Had we used scale to fill, the long side
        // would have fit as well. The difference between the scale to fill amount and
        // the amount we actually scaled the long side -- to preserve aspect ratio -- is the
        // additional scale. In this case, it's equal to the aspect ratio.
        //
        // Finally, the translation piece is here because we have to account for the parts
        // of the long side that were cut away due to the centered crop operation.
        toImage[5] = width / height;
        toImage[13] = (1 - width / height) / 2.0;
    }
    else if (_cropAndScaleOption == VROCropAndScaleOption::CoreML_Fit) {
        // Similar to FitCrop except the long side (Y) is fitted, and we must scale
        // and translate the short side (X) to maintain aspect ratio.
        toImage[0] = height / width;
        toImage[12] = (1 - height / width) / 2.0;
    }
    else if (_cropAndScaleOption == VROCropAndScaleOption::Viro_FitCropPad) {
        float cropX, cropY, cropWidth, cropHeight;
        image = performCropAndPad(image, &cropX, &cropY, &cropWidth, &cropHeight);
        needsRelease = true;
        
        // Derive the transform from vision space to *cropped* image space.
        // This is similar logic to CoreML_Fit.
        VROMatrix4f visionToCroppedImage;
        if (cropWidth > cropHeight) {
            // Add top and bottom bars
            visionToCroppedImage[5] = cropWidth / cropHeight;
            visionToCroppedImage[13] = (1 - cropWidth / cropHeight) / 2.0;
        } else {
            // Add left and right bars
            visionToCroppedImage[0] = cropHeight / cropWidth;
            visionToCroppedImage[12] = (1 - cropHeight / cropWidth) / 2.0;
        }
        
        // Then derive the transform from cropped image space to original
        // image space.
        VROMatrix4f croppedImageToImage;
        croppedImageToImage[0] = cropWidth / width;
        croppedImageToImage[5] = cropHeight / height;
        croppedImageToImage[12] = cropX / width;
        croppedImageToImage[13] = cropY / height;
        
        // Finally concatenate to get the vision to image transform.
        toImage = croppedImageToImage.multiply(visionToCroppedImage);
    }
    
    _visionToImageSpace = toImage;
    _imageToViewportSpace = toViewport;
    _startNeural = VROTimeCurrentMillis();
    
    // By wrapping the CVPixelBuffer in a CIImage, iOS will automatically convert from
    // YCbCr to RGB (note: this is undocumented, but works).
    CIImage *ciImage = [[CIImage alloc] initWithCVPixelBuffer:image];
    VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCIImage:ciImage
                                                                        orientation:orientation
                                                                            options:visionOptions];
    [handler performRequests:@[_visionRequest] error:nil];
    
    if (needsRelease) {
        CVBufferRelease(image);
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
    
    std::pair<VROVector3f, float> imageSpaceJoints[kNumBodyJoints];
    VROPoseFrame joints = convertHeatmap(heatmap, _cameraPosition, _visionToImageSpace, _imageToViewportSpace, imageSpaceJoints);
    _dynamicCropBox = deriveBounds(imageSpaceJoints);
    
    CGAffineTransform imageToViewport = CGAffineTransformMake(_imageToViewportSpace[0], _imageToViewportSpace[1], _imageToViewportSpace[4],
                                                              _imageToViewportSpace[5], _imageToViewportSpace[12], _imageToViewportSpace[13]);
    _dynamicCropBoxViewport = CGRectApplyAffineTransform(_dynamicCropBox, imageToViewport);
    if (_cameraPosition == VROCameraPosition::Front) {
        _dynamicCropBoxViewport.origin.x = 1.0 - _dynamicCropBoxViewport.size.width - _dynamicCropBoxViewport.origin.x;
    }
    
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

static int debugCropAndPadCurrentFrame = 0;
CVPixelBufferRef VROBodyTrackeriOS::performCropAndPad(CVPixelBufferRef image,
                                                      float *outCropX, float *outCropY,
                                                      float *outCropWidth, float *outCropHeight) {
    
    size_t width = CVPixelBufferGetWidth(image);
    size_t height = CVPixelBufferGetHeight(image);
    
    if (CGRectIsNull(_dynamicCropBox)) {
        *outCropWidth = (float) width;
        *outCropHeight = (float) height;
        
        // This method either returns a +1 retained pixelbuffer, or a new
        // pixelbuffer entirely
        CVBufferRetain(image);
        return image;
    }
    
    int bufferSize = (int) width * (int) height * 4;
    if (_cropScratchBuffer == nullptr || _cropScratchBufferLength < bufferSize) {
        pinfo("Creating new crop scratch buffer of size %d", bufferSize);
        if (_cropScratchBuffer != nullptr) {
            free(_cropScratchBuffer);
        }
        _cropScratchBuffer = (uint8_t *) malloc(bufferSize);
        _cropScratchBufferLength = bufferSize;
    }
    
    *outCropX = _dynamicCropBox.origin.x * (float) width;
    *outCropY = _dynamicCropBox.origin.y * (float) height;
    *outCropWidth = _dynamicCropBox.size.width * (float) width;
    *outCropHeight = _dynamicCropBox.size.height * (float) height;
    
    CVPixelBufferRef cropped = VROImagePreprocessor::cropAndResize(image, *outCropX, *outCropY, *outCropWidth, *outCropHeight,
                                                                   kVisionImageSize, _cropScratchBuffer);
    if (kDebugCropAndPadResult) {
        if (debugCropAndPadCurrentFrame == kDebugCropAndPadResultFrame) {
            VROImagePreprocessor::writeImageToPhotos(image);
            VROImagePreprocessor::writeImageToPhotos(cropped);
        }
        ++debugCropAndPadCurrentFrame;
    }
    return cropped;
}

CGRect VROBodyTrackeriOS::deriveBounds(const std::pair<VROVector3f, float> *imageSpaceJoints) {
    std::vector<std::vector<VROInferredBodyJoint>> VROPoseFrame;
    
    float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX;
    int numJointsFound = 0;
    
    for (int i = 0; i < kNumBodyJoints; i++) {
        if (imageSpaceJoints[i].second > kConfidenceThreshold) {
            numJointsFound++;
        
            VROVector3f position = imageSpaceJoints[i].first;
            minX = std::min(minX, position.x);
            minY = std::min(minY, position.y);
            maxX = std::max(maxX, position.x);
            maxY = std::max(maxY, position.y);
        }
    }
    
    if (numJointsFound < 6) {
        //pinfo("Not enough joints found for bounding box computation");
        return CGRectNull;
    }
    
    double timestamp = VROTimeCurrentMillis() / 1000.0;

    float x = _dynamicCropXFilter->filter(minX, timestamp);
    float y = _dynamicCropYFilter->filter(minY, timestamp);
    float width = _dynamicCropWidthFilter->filter(maxX - minX, timestamp);
    float height = _dynamicCropHeightFilter->filter(maxY - minY, timestamp);
    
    x -= width * kCropPaddingMultiplier;
    y -= height * kCropPaddingMultiplier;
    width *= (1 + 2 * kCropPaddingMultiplier);
    height *= (1 + 2 * kCropPaddingMultiplier);

    //pinfo("Filtered X %f, Y %f, width %f, height %f", minX, minY, maxX - minX, maxY - minY);
    return CGRectMake(x, y, width, height);
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
