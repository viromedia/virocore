//
//  VROHeadTracker.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/26/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef __CardboardSDK_iOS__HeadTracker__
#define __CardboardSDK_iOS__HeadTracker__

#include "OrientationEKF.h"

#import <CoreMotion/CoreMotion.h>
#import <simd/simd.h>
#import <GLKit/GLKit.h>

/*
 Continually tracks sensor data and returns the head rotation matrix.
 This head rotation matrix can be used to compute the view matrix for
 the renderer.
 */
class VROHeadTracker {
    
public:
    
    VROHeadTracker();
    virtual ~VROHeadTracker();
    
    void startTracking(UIInterfaceOrientation orientation);
    void stopTracking();
    matrix_float4x4 getHeadRotation();
    
    void updateDeviceOrientation(UIInterfaceOrientation orientation);
    bool isReady();
    
private:
    
    CMMotionManager *_motionManager;
    size_t _sampleCount;
    OrientationEKF *_tracker;
    
    GLKMatrix4 _worldToDeviceMatrix;
    GLKMatrix4 _IRFToWorldMatrix;
    GLKMatrix4 _correctedIRFToWorldMatrix;
    
    matrix_float4x4 _lastHeadRotation;
    NSTimeInterval _lastGyroEventTimestamp;
    
    bool _headingCorrectionComputed;
    
};

#endif
