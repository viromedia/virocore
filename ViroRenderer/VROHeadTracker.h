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

class VROHeadTracker {
    
public:
    
    VROHeadTracker();
    virtual ~VROHeadTracker();
    
    void startTracking(UIInterfaceOrientation orientation);
    void stopTracking();
    matrix_float4x4 getLastHeadView();
    
    void updateDeviceOrientation(UIInterfaceOrientation orientation);
    
    bool isNeckModelEnabled() {
        return _neckModelEnabled;
    }
    void setNeckModelEnabled(bool enabled) {
        _neckModelEnabled = enabled;
    }
    
    bool isReady();
    
private:
    
    CMMotionManager *_motionManager;
    size_t _sampleCount;
    OrientationEKF *_tracker;
    GLKMatrix4 _displayFromDevice;
    GLKMatrix4 _inertialReferenceFrameFromWorld;
    GLKMatrix4 _correctedInertialReferenceFrameFromWorld;
    matrix_float4x4 _lastHeadView;
    NSTimeInterval _lastGyroEventTimestamp;
    bool _headingCorrectionComputed;
    bool _neckModelEnabled;
    GLKMatrix4 _neckModelTranslation;
    float _orientationCorrectionAngle;

    const float _defaultNeckHorizontalOffset = 0.08f;
    const float _defaultNeckVerticalOffset = 0.075f;
    
};

#endif
