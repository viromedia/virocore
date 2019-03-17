//
//  VROBodyTrackeriOS.h
//  ViroKit
//
//  Created by vik.advani on 9/4/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROBodyTrackeriOS_h
#define VROBodyTrackeriOS_h

#include <map>
#include <atomic>
#include <mutex>
#include <memory>
#include "VROBodyTracker.h"
#include "VROCameraTexture.h"
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreML/CoreML.h>
#import <Vision/Vision.h>
#include "VROMatrix4f.h"

// Number of samples to collect when computing FPS
static const int kNeuralFPSMaxSamples = 100;

class VRODriver;
class VROPoseFilter;
class VROOneEuroFilterF;

enum class VROCropAndScaleOption {
    CoreML_Fill,
    CoreML_Fit,
    CoreML_FitCrop,
    Viro_FitCropPad,
};

class API_AVAILABLE(ios(11.0)) VROBodyTrackeriOS : public VROBodyTracker {
public:
    
    VROBodyTrackeriOS();
    virtual ~VROBodyTrackeriOS();
    
    bool initBodyTracking(VROCameraPosition position, std::shared_ptr<VRODriver> driver);
    void startBodyTracking();
    void stopBodyTracking();
    void update(const VROARFrame *frame);
    
    /*
     Sets the window period at which we sample points for dampening. If period == 0,
     no dampening will be applied.
     */
    void setDampeningPeriodMs(double period);
    double getDampeningPeriodMs() const;
    
    /*
     Get the dynamic crop box used for the last render.
     */
    CGRect getDynamicCropBox() const {
        return _dynamicCropBoxViewport;
    }
    
private:
    
    /*
     Core ML parameters.
     */
    MLModel *_model;
    VNCoreMLModel *_coreMLModel;
    VNCoreMLRequest *_visionRequest;
    
    VROCropAndScaleOption _cropAndScaleOption;
    uint8_t *_cropScratchBuffer;
    int _cropScratchBufferLength;
    
    /*
     True when tracking is running; e.g. images are being fed into CoreML.
     */
    bool _isTracking;
    
    /*
     The camera we are using for tracking (front or back).
     */
    VROCameraPosition _cameraPosition;
    
    /*
     Queue on which CoreML is run.
     */
    dispatch_queue_t _visionQueue;
    
    /*
     Transforms used to convert points from vision space (the CoreML input image
     space) into viewport points.
     
     This is a two-step operation: we have to move from vision space into image space
     (e.g. the space of the image before the cropping, scaling, and padding operations
     used for CoreML). Then we need to move from image space space into viewport space
     (inverting ARKit or similar display transforms).
     */
    VROMatrix4f _visionToImageSpace, _imageToViewportSpace;
    
    /*
     True when an image is being processed now.
     */
    bool _isProcessingImage;
    
    /*
     The image (along with its ARKit transform and orientation) that is queued to be
     fed into the CoreML model. The ARKit transform moves from image to viewport
     space.
     
     These are all protected by _imageMutex, as they are modified on the rendering thread
     but utlized on the _visionQueue thread.
     */
    CVPixelBufferRef _nextImage;
    VROMatrix4f _nextTransform;
    CGImagePropertyOrientation _nextOrientation;
    std::mutex _imageMutex;

    /*
     Filter used on pose data before sending to the delegate.
     */
    std::shared_ptr<VROPoseFilter> _poseFilter;
    
    /*
     Variables for neural engine FPS computation. Array of samples taken, index of
     next sample, and sum of samples so far.
     */
    int _fpsTickIndex = 0;
    uint64_t _fpsTickSum = 0;
    uint64_t _fpsTickArray[kNeuralFPSMaxSamples];
    
    /*
     FPS is measured in ticks, which are the number of nanoseconds
     between consecurive calls to processVisionResults().
     */
    uint64_t _nanosecondsLastFrame;
    bool _neuralEngineInitialized;
    
    /*
     Profiling variables for tracking time spent.
     */
    double _startNeural;
    double _startHeatmap;
    double _betweenImageTime;
    
    /*
     Dampening window milliseconds. If period is set to 0, no dampening will be applied.
     */
    double _dampeningPeriodMs;
    
    /*
     The dynamic cropping window, which is updated each frame to form a bounding
     box around the detected body.
     */
    CGRect _dynamicCropBox, _dynamicCropBoxViewport;
    std::shared_ptr<VROOneEuroFilterF> _dynamicCropXFilter, _dynamicCropYFilter, _dynamicCropWidthFilter, _dynamicCropHeightFilter;
    
    /*
     Move to track the next image, if one is available.
     */
    void nextImage();
    
    /*
     Initiate a VNImageRequest on the provided image, using the given inverse display transform.
     This method also derives the transform that will be needed to go back from vision space [0, 1] to
     viewport space [0, 1].
     
     Invoked on the visionQueue.
     */
    void trackImage(CVPixelBufferRef image, VROMatrix4f transform, CGImagePropertyOrientation orientation);
    
    /*
     Process the result of the last call to trackImage. This will convert the raw output from
     CoreML into the body joints, and then pass the joints through a filter and finally invoke the delegate
     on the rendering thread.
     
     Invoked on the visionQueue.
     */
    void processVisionResults(VNRequest *request, NSError *error);
    
    /*
     Converts the heatmap output from the given CoreML MLMultiArray into body joints in screen
     coordinates. The given transforms go from vision space [0, 1] to image space [0, 1], to
     normalized viewport space [0, 1].
     */
    static VROPoseFrame convertHeatmap(MLMultiArray *heatmap, VROCameraPosition cameraPosition,
                                       VROMatrix4f visionToImageSpace, VROMatrix4f imageToViewportSpace,
                                       std::pair<VROVector3f, float> *outImageSpaceJoints);
    
    /*
     Perform a crop and pad of the given image using the dynamic crop box,
     and return the result.
     */
    CVPixelBufferRef performCropAndPad(CVPixelBufferRef image, int *outCropX, int *outCropY,
                                       int *outCropWidth, int *outCropHeight);
    
    /*
     Derive the full body bounds from the given pose.
     */
    CGRect deriveBounds(const std::pair<VROVector3f, float> *imageSpaceJoints);
    
    /*
     Update and get FPS.
     */
    void updateFPS(uint64_t newTick);
    double getFPS() const;
};

#endif


