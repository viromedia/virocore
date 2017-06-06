//
//  VROHeadTracker.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/26/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROHeadTracker.h"
#include "VROMath.h"
#include "VROQuaternion.h"
#include "VROConvert.h"
#include "VROLog.h"

#define radiansToDegrees(x) (180/M_PI)*x

static const size_t kInitialSamplesToSkip = 10;

GLKMatrix4 GetRotateEulerMatrix(float x, float y, float z) {
    x *= (float)(M_PI / 180.0f);
    y *= (float)(M_PI / 180.0f);
    z *= (float)(M_PI / 180.0f);
    float cx = (float) cos(x);
    float sx = (float) sin(x);
    float cy = (float) cos(y);
    float sy = (float) sin(y);
    float cz = (float) cos(z);
    float sz = (float) sin(z);
    float cxsy = cx * sy;
    float sxsy = sx * sy;
    
    GLKMatrix4 matrix;
    matrix.m[0] = cy * cz;
    matrix.m[1] = -cy * sz;
    matrix.m[2] = sy;
    matrix.m[3] = 0.0f;
    matrix.m[4] = cxsy * cz + cx * sz;
    matrix.m[5] = -cxsy * sz + cx * cz;
    matrix.m[6] = -sx * cy;
    matrix.m[7] = 0.0f;
    matrix.m[8] = -sxsy * cz + sx * sz;
    matrix.m[9] = sxsy * sz + sx * cz;
    matrix.m[10] = cx * cy;
    matrix.m[11] = 0.0f;
    matrix.m[12] = 0.0f;
    matrix.m[13] = 0.0f;
    matrix.m[14] = 0.0f;
    matrix.m[15] = 1.0f;
    return matrix;
}

GLKMatrix4 GLMatrixFromRotationMatrix(CMRotationMatrix rotationMatrix) {
    GLKMatrix4 glRotationMatrix;
    
    glRotationMatrix.m00 = rotationMatrix.m11;
    glRotationMatrix.m01 = rotationMatrix.m12;
    glRotationMatrix.m02 = rotationMatrix.m13;
    glRotationMatrix.m03 = 0.0f;
    
    glRotationMatrix.m10 = rotationMatrix.m21;
    glRotationMatrix.m11 = rotationMatrix.m22;
    glRotationMatrix.m12 = rotationMatrix.m23;
    glRotationMatrix.m13 = 0.0f;
    
    glRotationMatrix.m20 = rotationMatrix.m31;
    glRotationMatrix.m21 = rotationMatrix.m32;
    glRotationMatrix.m22 = rotationMatrix.m33;
    glRotationMatrix.m23 = 0.0f;
    
    glRotationMatrix.m30 = 0.0f;
    glRotationMatrix.m31 = 0.0f;
    glRotationMatrix.m32 = 0.0f;
    glRotationMatrix.m33 = 1.0f;
    
    return glRotationMatrix;
}

VROHeadTracker::VROHeadTracker() :
    _worldToDeviceMatrix(GetRotateEulerMatrix(0.f, 0.f, 0.f)),

    // the inertial reference frame has z up and x forward, while the world has -z forward and x right
    _IRFToWorldMatrix(GetRotateEulerMatrix(-90.f, 0.f, 90.f)),
    _lastGyroEventTimestamp(0) {
        
    _motionManager = [[CMMotionManager alloc] init];
    _tracker = new OrientationEKF();
    
    _correctedIRFToWorldMatrix = _IRFToWorldMatrix;
    _lastHeadRotation = VROMatrix4f();
}

VROHeadTracker::~VROHeadTracker() {
    delete (_tracker);
}

void VROHeadTracker::startTracking(UIInterfaceOrientation orientation) {
    updateDeviceOrientation(orientation);
    
    _tracker->reset();
    
    _headingCorrectionComputed = false;
    _sampleCount = 0; // used to skip bad data when core motion starts
    
    if (_motionManager.isDeviceMotionAvailable) {
        [_motionManager startDeviceMotionUpdatesUsingReferenceFrame:CMAttitudeReferenceFrameXArbitraryZVertical];
        _sampleCount = kInitialSamplesToSkip + 1;
    }
}

void VROHeadTracker::stopTracking() {
    [_motionManager stopDeviceMotionUpdates];
}

bool VROHeadTracker::isReady() {
    bool isTrackerReady = (_sampleCount > kInitialSamplesToSkip);
    return isTrackerReady;
}

VROMatrix4f VROHeadTracker::getHeadRotation() {
    /*
     Get the tranpose (inverse) of the CoreMotion rotation matrix. This is what we'll
     rotate by. The CoreMotion matrix is in the "inertial reference frame" (IRF).
     */
    CMDeviceMotion *motion = _motionManager.deviceMotion;
    GLKMatrix4 rotationMatrix_IRF = GLKMatrix4Transpose(GLMatrixFromRotationMatrix(motion.attitude.rotationMatrix));
    
    if (!motion) {
        return _lastHeadRotation;
    }
    
    if (!isReady()) {
        return _lastHeadRotation;
    }

    /*
     I don't understand why this block is necessary.
     */
    if (!_headingCorrectionComputed) {
        // fix the heading by aligning world -z with the projection
        // of the device -z on the ground plane
        
        GLKMatrix4 deviceFromWorld = GLKMatrix4Multiply(rotationMatrix_IRF, _IRFToWorldMatrix);
        GLKMatrix4 worldFromDevice = GLKMatrix4Transpose(deviceFromWorld);
        
        GLKVector3 deviceForward = GLKVector3Make(0.f, 0.f, -1.f);
        GLKVector3 deviceForwardWorld = GLKMatrix4MultiplyVector3(worldFromDevice, deviceForward);
        
        if (fabsf(deviceForwardWorld.y) < 0.99f) {
            deviceForwardWorld.y = 0.f;  // project onto ground plane
            
            deviceForwardWorld = GLKVector3Normalize(deviceForwardWorld);
            
            // want to find R such that
            // deviceForwardWorld = R * [0 0 -1]'
            // where R is a rotation matrix about y, i.e.:
            //     [ c  0  s]
            // R = [ 0  1  0]
            //     [-s  0  c]
            
            float c = -deviceForwardWorld.z;
            float s = -deviceForwardWorld.x;
            // note we actually want to use the inverse, so
            // transpose when building
            GLKMatrix4 Rt = GLKMatrix4Make(
                  c, 0.f,  -s, 0.f,
                0.f, 1.f, 0.f, 0.f,
                  s, 0.f,   c, 0.f,
                0.f, 0.f, 0.f, 1.f );
            
            _correctedIRFToWorldMatrix = GLKMatrix4Multiply(_IRFToWorldMatrix, Rt);
        }
        
        _headingCorrectionComputed = true;
    }
    
    /*
     Convert the rotation matrix from the IRF to world coordinates by multiplying
     by the IRFtoWorld matrix.
     */
    GLKMatrix4 rotationMatrix_world = GLKMatrix4Multiply(rotationMatrix_IRF, _correctedIRFToWorldMatrix);
    GLKMatrix4 rotationMatrix_display = GLKMatrix4Multiply(_worldToDeviceMatrix, rotationMatrix_world);
    
    _lastHeadRotation = VROConvert::toMatrix4f(rotationMatrix_display);
    return _lastHeadRotation;
}

void VROHeadTracker::updateDeviceOrientation(UIInterfaceOrientation orientation) {
    if (orientation == UIInterfaceOrientationLandscapeLeft) {
        _worldToDeviceMatrix = GetRotateEulerMatrix(0.f, 0.f, 90.f);
    }
    else if (orientation == UIInterfaceOrientationLandscapeRight) {
        _worldToDeviceMatrix = GetRotateEulerMatrix(0.f, 0.f, -90.f);
    }
    else {
        _worldToDeviceMatrix = GLKMatrix4Identity;
    }
}
