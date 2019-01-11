//
//  VROObjectRecognizeriOS.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/10/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROObjectRecognizeriOS_hpp
#define VROObjectRecognizeriOS_hpp

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
    
    void trackWithVision(CVPixelBufferRef image, VROMatrix4f transform, VROCameraOrientation orientation);
    
private:
    
    MLModel *_model;
    VNCoreMLModel *_coreMLModel;
    VNCoreMLRequest *_coreMLRequest;
    double _lastTimestamp;
    int32_t _fps;
    dispatch_queue_t bodyMeshingQueue;
    
    VROMatrix4f _transform;
    
    static CVPixelBufferRef convertImage(CVImageBufferRef image);
    static CVPixelBufferRef rotateImage(CVPixelBufferRef image, uint8_t rotation, size_t resultWidth, size_t resultHeight);
    static CVPixelBufferRef transformImage(CVPixelBufferRef image, CGAffineTransform transform);
    static std::map<std::string, VRORecognizedObject> convertHeatmap(MLMultiArray *heatmap, VROMatrix4f transform);
    
    // Debug method
    void writeImageToDisk(CVPixelBufferRef imageBuffer);
    
};

#endif /* VROObjectRecognizeriOS_hpp */
