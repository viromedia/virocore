//
//  VROVisionEngine.cpp
//  ViroKit
//
//  Created by Raj Advani on 5/22/19.
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

#include "VROVisionEngine.h"
#include "VROLog.h"
#include "VROTime.h"
#include "VROMath.h"
#include <Accelerate/Accelerate.h>
#include "VROImagePreprocessor.h"
#include "VROARFrameiOS.h"
#include "VROARFrameInertial.h"
#include "VROOneEuroFilter.h"

// For deriving dynamic crop box from joints
#include "VROBodyTracker.h"
#include <mutex>

#define VRO_PROFILE_NEURAL_ENGINE 0

static const float kConfidenceThreshold = 0.15;

// Set to true to save the crop and padding result of one of the early
// frames to the Photo Library, for debubbing.
static bool kDebugCropAndPadResult = false;
static const int kDebugCropAndPadResultFrame = 30;

// Parameterization for the One Euro filter used when dynamic cropping.
static const double kCropEuroFilterFrequency = 60;
static const double kCropEuroFilterDCutoff = 1;
static const double kCropEuroFilterBeta = 1.0;
static const double kCropEuroFilterFCMin = 1.7;

// The amount of additional padding (as percentage of the body bounds) to
// add to the dynamic crop box.
static const double kCropPaddingMultiplierX = 0.40;
static const double kCropPaddingMultiplierY = 0.25;

// The interpolation amount to use when moving from an old crop box to a
// newly found one. Higher number means we'll move more quickly to the
// latest crop box, while lower values prioritize more smoothing.
static const float kCropBoxSmoothingFactor = 0.2;

// The dynamic crop box is thrown out in favor of the entire screen if we
// find fewer than these many joints.
static const int kCropMinimumJoints = 6;

// Amount to subtract from the crop region to prevent CoreML bugs when
// using region of interest.
static const double kRegionOfInterestCroppingEpsilon = 0.001;

// When using region of interest, if our cropping box is this close (in
// normalized image coordinates) to the full screen in both dimensions,
// then just use the full screen.
static const double kRegionOfInterestRoundingEpsilon = 0.05;

#pragma mark - Initialization

VROVisionEngine::VROVisionEngine(MLModel *model, int imageSize, VROCameraPosition position,
                                 VROCropAndScaleOption cropAndScaleOption) {
    _visionQueue = dispatch_queue_create("com.viro.visionQueue", DISPATCH_QUEUE_SERIAL);

    _imageSize = imageSize;
    _fpsTickIndex = 0;
    _fpsTickSum = 0;
    _neuralEngineInitialized = false;
    _processingImage = nil;
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
    
    _model = model;
    _cameraPosition = position;
    _cropAndScaleOption = VROCropAndScaleOption::Viro_RegionOfInterest;
    
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
        case VROCropAndScaleOption::Viro_FitCropPad:
            // For Viro cropping, disable CoreML's cropping and scaling by setting it to ScaleFill
            // Note: this cropping does not yet work with the back-facing camera
            _visionRequest.imageCropAndScaleOption = VNImageCropAndScaleOptionScaleFill;
            break;
        case VROCropAndScaleOption::Viro_RegionOfInterest:
            _visionRequest.imageCropAndScaleOption = VNImageCropAndScaleOptionScaleFit;
            break;
        default:
            pabort();
    }
}

VROVisionEngine::~VROVisionEngine() {
    if (_cropScratchBuffer != nullptr) {
        free(_cropScratchBuffer);
    }
}

#pragma mark - Renderer Thread

void VROVisionEngine::update(const VROARFrame *frame) {
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
    std::weak_ptr<VROVisionEngine> tracker_w = std::dynamic_pointer_cast<VROVisionEngine>(shared_from_this());
    dispatch_async(_visionQueue, ^{
        std::shared_ptr<VROVisionEngine> tracker = tracker_w.lock();
        if (tracker) {
            tracker->nextImage();
        }
    });
}

#pragma mark - Vision Queue (pre-processing for CoreML)

// Invoked on the _visionQueue
void VROVisionEngine::nextImage() {
#if VRO_PROFILE_NEURAL_ENGINE
    NSLog(@"Starting neural engine frame");
#endif
    
    VROMatrix4f transform;
    CGImagePropertyOrientation orientation;
    {
        std::lock_guard<std::mutex> lock(_imageMutex);
        // Only process one image at a time
        if (_processingImage || !_nextImage) {
            return;
        }
        
        _processingImage = CVBufferRetain(_nextImage);
        CVBufferRelease(_nextImage);
        _nextImage = nil;
        
        transform = _nextTransform;
        orientation = _nextOrientation;
    }
    
#if VRO_PROFILE_NEURAL_ENGINE
    NSLog(@"   Idle time %f", VROTimeCurrentMillis() - _betweenImageTime);
#endif
    
    trackImage(_processingImage, transform, orientation);
    {
        std::lock_guard<std::mutex> lock(_imageMutex);
        CVBufferRelease(_processingImage);
        _processingImage = nil;
    }
    _betweenImageTime = VROTimeCurrentMillis();
    
    // Use dispatch_async to prevent recursion overflow
    std::weak_ptr<VROVisionEngine> tracker_w = std::dynamic_pointer_cast<VROVisionEngine>(shared_from_this());
    dispatch_async(_visionQueue, ^{
        std::shared_ptr<VROVisionEngine> tracker = tracker_w.lock();
        if (tracker) {
            tracker->nextImage();
        }
    });
}

// Invoked on the _visionQueue
void VROVisionEngine::trackImage(CVPixelBufferRef image, VROMatrix4f transform, CGImagePropertyOrientation orientation) {
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
        int cropX, cropY, cropWidth, cropHeight;
        image = performCropAndPad(image, &cropX, &cropY, &cropWidth, &cropHeight);
        needsRelease = true;
        
        // Derive the transform from vision space to *cropped* image space.
        // This is similar logic to CoreML_Fit.
        VROMatrix4f visionToCroppedImage;
        if (cropWidth > cropHeight) {
            float scale = (float) cropWidth / (float) cropHeight;
            
            // Add top and bottom bars
            visionToCroppedImage[5] = scale;
            visionToCroppedImage[13] = (1 - scale) / 2.0;
        } else {
            float scale = (float) cropHeight / (float) cropWidth;
            
            // Add left and right bars
            visionToCroppedImage[0] = scale;
            visionToCroppedImage[12] = (1 - scale) / 2.0;
        }
        
        // Then derive the transform from cropped image space to original
        // image space.
        VROMatrix4f croppedImageToImage;
        croppedImageToImage[0] = (float) cropWidth / width;
        croppedImageToImage[5] = (float) cropHeight / height;
        croppedImageToImage[12] = (float) cropX / width;
        croppedImageToImage[13] = (float) cropY / height;
        
        // Finally concatenate to get the vision to image transform.
        toImage = croppedImageToImage.multiply(visionToCroppedImage);
    }
    else if (_cropAndScaleOption == VROCropAndScaleOption::Viro_RegionOfInterest) {
        float cropX, cropY, cropWidth, cropHeight;
        
        if (CGRectIsNull(_dynamicCropBox)) {
            cropX = 0;
            cropY = 0;
            cropWidth = 1;
            cropHeight = 1;
        } else {
            cropX = _dynamicCropBox.origin.x;
            cropY = _dynamicCropBox.origin.y;
            cropWidth  = _dynamicCropBox.size.width;
            cropHeight = _dynamicCropBox.size.height;
            
            cropX = clamp(cropX, kRegionOfInterestCroppingEpsilon, 1.0 - kRegionOfInterestCroppingEpsilon);
            cropY = clamp(cropY, kRegionOfInterestCroppingEpsilon, 1.0 - kRegionOfInterestCroppingEpsilon);
            cropWidth  = clamp(cropWidth,  0, 1.0 - kRegionOfInterestCroppingEpsilon - cropX);
            cropHeight = clamp(cropHeight, 0, 1.0 - kRegionOfInterestCroppingEpsilon - cropY);
        }
        
        // Derive the transform from vision space to *cropped* image space.
        // This is similar logic to CoreML_Fit. Note CoreML always fits the
        // long side, even if our crop region's width is greater than its
        // height.
        VROMatrix4f visionToCroppedImage;
        float scale = (float) (cropHeight * height) / (float) (cropWidth * width);
        
        // Add left and right bars
        visionToCroppedImage[0] = scale;
        visionToCroppedImage[12] = (1 - scale) / 2.0;
        
        // Then derive the transform from cropped image space to original
        // image space.
        VROMatrix4f croppedImageToImage;
        croppedImageToImage[0] = cropWidth;
        croppedImageToImage[5] = cropHeight;
        croppedImageToImage[12] = cropX;
        croppedImageToImage[13] = cropY;
        
        // Finally concatenate to get the vision to image transform.
        toImage = croppedImageToImage.multiply(visionToCroppedImage);
        
        // If the region of interest is close to the full screen, then just use the
        // full screen (for efficiency and to prevent artifacts).
        if (fabs(1.0 - cropWidth) < kRegionOfInterestRoundingEpsilon &&
            fabs(1.0 - cropHeight) < kRegionOfInterestRoundingEpsilon) {
            
            cropX = 0;
            cropY = 0;
            cropWidth = 1;
            cropHeight = 1;
        }
        
        // Set the region of interest to the crop region. The fabs is to prevent the value
        // -0.000, which for unknown reasons breaks CoreML.
        _visionRequest.regionOfInterest = CGRectMake(cropX, fabs(1 - cropY - cropHeight), cropWidth, cropHeight);
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
static int debugCropAndPadCurrentFrame = 0;
CVPixelBufferRef VROVisionEngine::performCropAndPad(CVPixelBufferRef image,
                                                    int *outCropX, int *outCropY,
                                                    int *outCropWidth, int *outCropHeight) {
    size_t width = CVPixelBufferGetWidth(image);
    size_t height = CVPixelBufferGetHeight(image);
    
    if (CGRectIsNull(_dynamicCropBox)) {
        *outCropX = 0;
        *outCropY = 0;
        *outCropWidth  = (int) width;
        *outCropHeight = (int) height;
        
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
    
    *outCropX = _dynamicCropBox.origin.x * width;
    *outCropY = _dynamicCropBox.origin.y * height;
    *outCropWidth = _dynamicCropBox.size.width * width;
    *outCropHeight = _dynamicCropBox.size.height * height;
    
    *outCropX = clamp(*outCropX, 0, width);
    *outCropY = clamp(*outCropY, 0, height);
    *outCropWidth  = clamp(*outCropWidth,  0, (int) width  - *outCropX);
    *outCropHeight = clamp(*outCropHeight, 0, (int) height - *outCropY);
    
    CVPixelBufferRef cropped = VROImagePreprocessor::cropAndResize(image, *outCropX, *outCropY, *outCropWidth, *outCropHeight,
                                                                   _imageSize, _cropScratchBuffer);
    if (kDebugCropAndPadResult) {
        if (debugCropAndPadCurrentFrame >= kDebugCropAndPadResultFrame &&
            debugCropAndPadCurrentFrame < kDebugCropAndPadResultFrame + 1) {
            
            VROImagePreprocessor::writeImageToPhotos(VROImagePreprocessor::convertYCbCrToRGB(image));
            VROImagePreprocessor::writeImageToPhotos(VROImagePreprocessor::convertYCbCrToRGB(cropped));
        }
        ++debugCropAndPadCurrentFrame;
    }
    return cropped;
}

#pragma mark - Vision Queue (post-processing CoreML output)

// Invoked on the _visionQueue
void VROVisionEngine::processVisionResults(VNRequest *request, NSError *error) {
#if VRO_PROFILE_NEURAL_ENGINE
    NSLog(@"   Neural engine time %f", VROTimeCurrentMillis() - _startNeural);
#endif
    _startPostProcessing = VROTimeCurrentMillis();
    
    NSArray *array = [request results];
    VNCoreMLFeatureValueObservation *topResult = (VNCoreMLFeatureValueObservation *)(array[0]);
    
    std::shared_ptr<VROVisionEngineDelegate> delegate = _delegate_w.lock();
    if (delegate) {
        std::vector<std::pair<VROVector3f, float>> imageSpaceJoints = delegate->processVisionOutput(topResult, _cameraPosition, _visionToImageSpace, _imageToViewportSpace);
        
        if (!imageSpaceJoints.empty()) {
             _dynamicCropBox = deriveBoundsSmooth(imageSpaceJoints.data());
            
            CGAffineTransform imageToViewport = CGAffineTransformMake(_imageToViewportSpace[0], _imageToViewportSpace[1], _imageToViewportSpace[4],
                                                                      _imageToViewportSpace[5], _imageToViewportSpace[12], _imageToViewportSpace[13]);
            _dynamicCropBoxViewport = CGRectApplyAffineTransform(_dynamicCropBox, imageToViewport);
            if (_cameraPosition == VROCameraPosition::Front) {
                _dynamicCropBoxViewport.origin.x = 1.0 - _dynamicCropBoxViewport.size.width - _dynamicCropBoxViewport.origin.x;
            }
        }
    }
    
#if VRO_PROFILE_NEURAL_ENGINE
    NSLog(@"   Post-processing time %f", VROTimeCurrentMillis() - _startPostProcessing);
#endif
    
    // Compute FPS
    uint64_t nanosecondsThisFrame = VRONanoTime();
    uint64_t tick = nanosecondsThisFrame - _nanosecondsLastFrame;
    _nanosecondsLastFrame = nanosecondsThisFrame;
    updateFPS(tick);
    
#if VRO_PROFILE_NEURAL_ENGINE
    NSLog(@"Neural Engine FPS %f\n", getFPS());
#endif
}

CGRect VROVisionEngine::deriveBounds(const std::pair<VROVector3f, float> *imageSpaceJoints) {
    std::vector<std::vector<VROInferredBodyJoint>> VROPoseFrame;
    
    float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX;
    std::vector<bool> jointsFound(kNumBodyJoints, false);
    int numJointsFound = 0;
    
    for (int i = 0; i < kNumBodyJoints; i++) {
        if (imageSpaceJoints[i].second > kConfidenceThreshold) {
            ++numJointsFound;
            
            VROVector3f position = imageSpaceJoints[i].first;
            minX = std::min(minX, position.x);
            minY = std::min(minY, position.y);
            maxX = std::max(maxX, position.x);
            maxY = std::max(maxY, position.y);
            
            jointsFound[i] = true;
        }
    }
    
    if (numJointsFound < kCropMinimumJoints) {
        return CGRectNull;
    }
    
    double timestamp = VROTimeCurrentMillis() / 1000.0;
    
    float x = _dynamicCropXFilter->filter(minX, timestamp);
    float y = _dynamicCropYFilter->filter(minY, timestamp);
    float width = _dynamicCropWidthFilter->filter(maxX - minX, timestamp);
    float height = _dynamicCropHeightFilter->filter(maxY - minY, timestamp);
    
    x -= width * kCropPaddingMultiplierX;
    y -= height * kCropPaddingMultiplierY;
    width *= (1 + 2 * kCropPaddingMultiplierX);
    height *= (1 + 2 * kCropPaddingMultiplierY);
    
    // Expand the crop box in the direction of unfound joints, to increase the chance of them
    // being found in the next frame
    if (!jointsFound[(int) VROBodyJointType::LeftWrist] || !jointsFound[(int) VROBodyJointType::LeftAnkle]) {
        float expansion = width * kCropPaddingMultiplierX;
        x -= expansion;
        width += 2 * expansion;
    }
    if (!jointsFound[(int) VROBodyJointType::Top]) {
        float expansion = height * kCropPaddingMultiplierY;
        y -= expansion;
        height += 2 * expansion;
    }
    if (!jointsFound[(int) VROBodyJointType::RightWrist] || !jointsFound[(int) VROBodyJointType::RightAnkle]) {
        float expansion = width * kCropPaddingMultiplierX;
        x -= expansion;
        width += 2 * expansion;
    }
    if (!jointsFound[(int) VROBodyJointType::LeftAnkle] && !jointsFound[(int) VROBodyJointType::RightAnkle]) {
        float expansion = height * 4 * kCropPaddingMultiplierY;
        y -= expansion;
        height += 2 * expansion;
    }
    
    // If the pelvis was visible, then ensure the bounds have a height of at least
    // pelvis to legs in either direction of the pelvis.
    if (jointsFound[(int) VROBodyJointType::Pelvis]) {
        float maxPelvisAnkleDistance = 0;
        VROVector3f pelvisPosition = imageSpaceJoints[(int) VROBodyJointType::Pelvis].first;
        
        if (jointsFound[(int) VROBodyJointType::LeftAnkle]) {
            maxPelvisAnkleDistance = fmax(maxPelvisAnkleDistance,
                                          pelvisPosition.distance(imageSpaceJoints[(int) VROBodyJointType::LeftAnkle].first));
        }
        if (jointsFound[(int) VROBodyJointType::RightAnkle]) {
            maxPelvisAnkleDistance = fmax(maxPelvisAnkleDistance,
                                          pelvisPosition.distance(imageSpaceJoints[(int) VROBodyJointType::RightAnkle].first));
        }
        
        if (pelvisPosition.y - maxPelvisAnkleDistance < y) {
            float yAdj = pelvisPosition.y - maxPelvisAnkleDistance;
            height += (y - yAdj);
            y = yAdj;
        }
    }
    
    x = fmax(x, 0);
    y = fmax(y, 0);
    width = fmin(width, 1 - x);
    height = fmin(height, 1 - y);
    
    return CGRectMake(x, y, width, height);
}

CGRect VROVisionEngine::deriveBoundsSmooth(const std::pair<VROVector3f, float> *imageSpaceJoints) {
    CGRect newBounds = deriveBounds(imageSpaceJoints);
    if (CGRectIsNull(_dynamicCropBox)) {
        return newBounds;
    }
    
    float interpolation = kCropBoxSmoothingFactor;
    float x = VROMathInterpolate(interpolation, 0, 1, _dynamicCropBox.origin.x, newBounds.origin.x);
    float y = VROMathInterpolate(interpolation, 0, 1, _dynamicCropBox.origin.y, newBounds.origin.y);
    float width = VROMathInterpolate(interpolation, 0, 1, _dynamicCropBox.size.width, newBounds.size.width);
    float height = VROMathInterpolate(interpolation, 0, 1, _dynamicCropBox.size.height, newBounds.size.height);
    
    x = fmax(x, 0);
    y = fmax(y, 0);
    width = fmin(width, 1 - x);
    height = fmin(height, 1 - y);
    
    return CGRectMake(x, y, width, height);
}

#pragma mark - FPS Computation

void VROVisionEngine::updateFPS(uint64_t newTick) {
    // Simple moving average: subtract value falling off, and add new value
    
    _fpsTickSum -= _fpsTickArray[_fpsTickIndex];
    _fpsTickSum += newTick;
    _fpsTickArray[_fpsTickIndex] = newTick;
    
    if (++_fpsTickIndex == kNeuralFPSMaxSamples) {
        _fpsTickIndex = 0;
    }
}

double VROVisionEngine::getFPS() const {
    double averageNanos = ((double) _fpsTickSum) / kNeuralFPSMaxSamples;
    return 1.0 / (averageNanos / (double) 1e9);
}
