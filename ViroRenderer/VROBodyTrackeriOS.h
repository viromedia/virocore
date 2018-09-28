//
//  VROBodyTrackeriOS.h
//  ViroKit
//
//  Created by vik.advani on 9/4/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROBodyTrackeriOS_h
#define VROBodyTrackeriOS_h

#include <memory>
#include "VROCameraTexture.h"
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreML/CoreML.h>
#import <Vision/Vision.h>

class VRODriver;

enum class VROBodyMeshingJoints {
    kTop = 0,
    kNeck = 1,
    kRightShoulder = 2,
    kRightElbow = 3,
    kRightWrist = 4,
    kLeftShoulder = 5,
    kLeftElbow =6,
    kLeftWrist = 7,
    kRightHip = 8,
    kRightKnee = 9,
    kRightAnkle = 10,
    kLeftHip = 11,
    kLeftKnee = 12,
    kLeftAngle = 13,
};

class BodyPoint  {
public:
    BodyPoint(CGPoint point, double confidence) {
        _point = point;
        _confidence = confidence;
    }

    CGPoint _point;
    double _confidence;
};

@interface BodyPointImpl: NSObject
    @property (readwrite, nonatomic) CGPoint _point;
    @property (readwrite, nonatomic) double _confidence;
@end

class VROBodyTrackerDelegate {
public:
    virtual void onBodyJointsFound(NSDictionary *joints) = 0;
};

class VROBodyTrackeriOS {
    
public:
    
    VROBodyTrackeriOS();
    virtual ~VROBodyTrackeriOS() {}
    
    bool initBodyTracking(VROCameraPosition position, std::shared_ptr<VRODriver> driver);
    bool isBodyPointConfidenceLessThan(BodyPointImpl *bodyPoint, float confidence);
    void startBodyTracking();
    void stopBodyTracking();
    void processBuffer(CVPixelBufferRef sampleBuffer);
    
    void setDelegate(std::shared_ptr<VROBodyTrackerDelegate> delegate) {
        auto autoWeakDelegate = delegate;
        _bodyMeshDelegateWeak = autoWeakDelegate;
    }
    
private:
    
    MLModel *_model;
    VNCoreMLModel *_coreMLModel;
    VNCoreMLRequest *_coreMLRequest;
    CMTime _lastTimestamp;
    int32_t _fps;
    dispatch_queue_t bodyMeshingQueue;
    std::weak_ptr<VROBodyTrackerDelegate> _bodyMeshDelegateWeak;
    CVPixelBufferRef rotatedBuffer;
    
    CVPixelBufferRef convertImage(CVImageBufferRef imageBuffer);
    NSDictionary *convert(MLMultiArray *heatmap);
    
    void writeImageToDisk(CVPixelBufferRef imageBuffer);
    void printBodyPoint(NSDictionary *bodyPoints, VROBodyMeshingJoints jointType);

};

#endif


