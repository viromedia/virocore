//
//  VROBodyTrackeriOS.h
//  ViroKit
//
//  Created by vik.advani on 9/4/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROBodyTrackeriOS_h
#define VROBodyTrackeriOS_h

#include "VROBodyTracker.h"
#include <map>
#include "VROCameraTexture.h"
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreML/CoreML.h>
#import <Vision/Vision.h>
#include "VROMatrix4f.h"
#include <atomic>
#include <mutex>


// Number of samples to collect when computing FPS
static const int kNeuralFPSMaxSamples = 100;

class VRODriver;

class VROBodyTrackeriOS : public VROBodyTracker {
    
public:
    
    VROBodyTrackeriOS();
    virtual ~VROBodyTrackeriOS() {}
    
    bool initBodyTracking(VROCameraPosition position, std::shared_ptr<VRODriver> driver);
    void startBodyTracking();
    void stopBodyTracking();
    void update(const VROARFrame &frame);
    
private:
    
    MLModel *_model;
    VNCoreMLModel *_coreMLModel;
    VNCoreMLRequest *_visionRequest;
    
    dispatch_queue_t _visionQueue;
    VROMatrix4f _transform;
    bool _isProcessingImage;
    bool _isTracking;
    
    CVPixelBufferRef _nextImage;
    VROMatrix4f _nextTransform;
    VROCameraOrientation _nextOrientation;
    
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
    
    std::mutex _imageMutex;
    
    /*
     Profiling variables for tracking time spent.
     */
    double _startNeural;
    double _startHeatmap;
    double _betweenImageTime;
    
    /*
     Move to track the next image, if one is available.
     */
    void nextImage();
    
    void trackImage(CVPixelBufferRef image, VROMatrix4f transform, VROCameraOrientation orientation);
    void processVisionResults(VNRequest *request, NSError *error);
    static std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> convertHeatmap(MLMultiArray *heatmap, VROMatrix4f transform);

    void updateFPS(uint64_t newTick);
    double getFPS() const;
};

#endif


