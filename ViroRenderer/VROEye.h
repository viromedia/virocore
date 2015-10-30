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

/*
 An eye consists of a view matrix, a projection matrix, a viewport, and a
 field of view. These completely define the "camera" of an eye.
 */
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
        _perspective(matrix_identity_float4x4),
        _projectionChanged(true),
        _lastZNear(0),
        _lastZFar(0) {
        
    }
    
    ~VROEye() {}
    
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
        
        _perspective = _fov.toPerspectiveMatrix(zNear, zFar);
        _lastZNear = zNear;
        _lastZFar = zFar;
        _projectionChanged = false;
        
        return _perspective;
    }
    
    void setFOV(float left, float right, float bottom, float top) {
        _fov.setLeft(left);
        _fov.setRight(right);
        _fov.setBottom(bottom);
        _fov.setTop(top);
        
        _projectionChanged = true;
    }
    
    const VROFieldOfView &getFOV() const {
        return _fov;
    }
    
    void setViewport(float x, float y, float width, float height) {
        _viewport.setViewport(x, y, width, height);
    }
    
    const VROViewport &getViewport() const {
        return _viewport;
    }
    
private:
    
    Type _type;
    
    matrix_float4x4 _eyeView;
    matrix_float4x4 _perspective;

    VROViewport _viewport;
    VROFieldOfView _fov;
    
    bool _projectionChanged;
    float _lastZNear;
    float _lastZFar;
};

#endif /* VROEye_h */
