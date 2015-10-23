//
//  VROEye.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/23/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROEye_h
#define VROEye_h

#include <stdio.h>
#include <simd/simd.h>
#include "VROViewport.h"
#include "VROFieldOfView.h"

class VROEye {
    
public:
    
    typedef enum {
        TypeMonocular = 0,
        TypeLeft = 1,
        TypeRight = 2
    } Type;
    
    VROEye(const Type eye) :
        _type(eye),
        _eyeView(matrix_identity_float4x4),
        _projectionChanged(true),
        _perspective(matrix_identity_float4x4),
        _lastZNear(0),
        _lastZFar(0) {
        
        _viewport = new VROViewport();
        _fov = new VROFieldOfView();
    }
    
    ~VROEye() {
        if (_viewport != nullptr) {
            delete (_viewport);
        }
        if (_fov != nullptr) {
            delete (_fov);
        }
    }
    
    Type getType() const {
        return _type;
    }
    
    matrix_float4x4 getEyeView() const {
        return _eyeView;
    }
    
    void setEyeView(matrix_float4x4 eyeView) {
        _eyeView = eyeView;
    }
    
    matrix_float4x4 perspective(float zNear, float zFar) {
        if (!_projectionChanged && _lastZNear == zNear && _lastZFar == zFar) {
            return _perspective;
        }
        
        _perspective = _fov->toPerspectiveMatrix(zNear, zFar);
        _lastZNear = zNear;
        _lastZFar = zFar;
        _projectionChanged = false;
        
        return _perspective;
    }
    
    VROViewport *getViewport() const {
        return _viewport;
    }
    
    VROFieldOfView *getFOV() const {
        return _fov;
    }
    
    void setProjectionChanged() {
        _projectionChanged = true;
    }
    
private:
    
    Type _type;
    matrix_float4x4 _eyeView;
    VROViewport *_viewport;
    VROFieldOfView *_fov;
    bool _projectionChanged;
    matrix_float4x4 _perspective;
    float _lastZNear;
    float _lastZFar;
};

#endif /* VROEye_h */
