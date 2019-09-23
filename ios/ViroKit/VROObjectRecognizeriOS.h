//
//  VROObjectRecognizeriOS.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/10/19.
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

    void update(const VROARFrame *frame);
    
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
