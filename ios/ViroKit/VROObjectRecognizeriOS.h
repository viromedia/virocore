//
//  VROObjectRecognizeriOS.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/10/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROObjectRecognizeriOS_h
#define VROObjectRecognizeriOS_h

#include "VROObjectRecognizer.h"
#include <map>
#include "VROCameraTexture.h"
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreML/CoreML.h>
#import <Vision/Vision.h>
#include "VROMatrix4f.h"

class VRODriver;

class VROObjectRecognizeriOS : public VROObjectRecognizer {
    
public:
    
    VROObjectRecognizeriOS();
    virtual ~VROObjectRecognizeriOS() {}
    
    bool initObjectTracking(VROCameraPosition position, std::shared_ptr<VRODriver> driver);
    void startObjectTracking();
    void stopObjectTracking();

    void update(const VROARFrame &frame);
    
private:
    
    MLModel *_model;
    VNCoreMLModel *_coreMLModel;
    VNCoreMLRequest *_visionRequest;
    
    dispatch_queue_t _visionQueue;
    VROMatrix4f _transform;
    CVPixelBufferRef _currentImage;
    
    void trackCurrentImage(VROMatrix4f transform, VROCameraOrientation orientation);
    void processVisionResults(VNRequest *request, NSError *error);
    
};

#endif /* VROObjectRecognizeriOS_h */
