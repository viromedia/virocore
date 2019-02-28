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

class VROBodyTrackeriOS : public VROBodyTracker {
public:
    
    VROBodyTrackeriOS();
    virtual ~VROBodyTrackeriOS() {}
    
    bool initBodyTracking(VROCameraPosition position, std::shared_ptr<VRODriver> driver);
    void startBodyTracking();
    void stopBodyTracking();
    void update(const VROARFrame &frame);
    
    /*
     Sets the window period at which we sample points for dampening. If period == 0,
     no dampening will be applied.
     */
    void setDampeningPeriodMs(double period);
    double getDampeningPeriodMs() const;
    
private:
    
    /*
     Core ML parameters.
     */
    MLModel *_model;
    VNCoreMLModel *_coreMLModel;
    VNCoreMLRequest *_visionRequest;
    VNImageCropAndScaleOption _cropAndScaleOption;
    
    /*
     True when tracking is running; e.g. images are being fed into CoreML.
     */
    bool _isTracking;
    
    /*
     Queue on which CoreML is run.
     */
    dispatch_queue_t _visionQueue;
    
    /*
     Transform used to convert points on the image input into CoreML
     (currently) into viewport points. This moves from CoreML image space into
     viewport space.
     */
    VROMatrix4f _transform;
    
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
    VROCameraOrientation _nextOrientation;
    std::mutex _imageMutex;

    /*
     Filter used on pose data before sending to the delegate.
     */
    std::shared_ptr<VROPoseFilter> _poseFilterA;
    std::shared_ptr<VROPoseFilter> _poseFilterB;
    
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
     Move to track the next image, if one is available.
     */
    void nextImage();
    
    void trackImage(CVPixelBufferRef image, VROMatrix4f transform, VROCameraOrientation orientation);
    void processVisionResults(VNRequest *request, NSError *error);
    static std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> convertHeatmap(MLMultiArray *heatmap, VROMatrix4f transform);

    /*
     Update and get FPS.
     */
    void updateFPS(uint64_t newTick);
    double getFPS() const;
};

#endif


