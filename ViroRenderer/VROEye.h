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
#include "VROMatrix4f.h"
#include "VROViewport.h"
#include "VROFieldOfView.h"

enum class VROEyeType {
    Left = 0,
    Right = 1,
    Monocular = 2
};

/*
 An eye consists of a view matrix, a projection matrix, a viewport, and a
 field of view. These completely define the "camera" of an eye.
 */
class VROEye {
    
public:
    
    VROEye(const VROEyeType type) :
        _type(type) {
        
    }
    
    ~VROEye() {}
    
    VROEyeType getType() const {
        return _type;
    }
    
    VROMatrix4f getEyeView() const {
        return _eyeView;
    }
    
    void setEyeView(VROMatrix4f eyeView) {
        _eyeView = eyeView;
    }
    
    VROMatrix4f getPerspectiveMatrix() const {
        return _perspective;
    }
    
    void setFOV(float left, float right, float bottom, float top, float zNear, float zFar) {
        _fov.setLeft(left);
        _fov.setRight(right);
        _fov.setBottom(bottom);
        _fov.setTop(top);
        _perspective = _fov.toPerspectiveProjection(zNear, zFar);
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
    
    VROEyeType _type;
    
    VROMatrix4f _eyeView;
    VROMatrix4f _perspective;

    VROViewport _viewport;
    VROFieldOfView _fov;
    
};

#endif /* VROEye_h */
