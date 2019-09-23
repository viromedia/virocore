//
//  VROEye.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/23/15.
//  Copyright © 2015 Viro Media. All rights reserved.
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
    
    static std::string toString(VROEyeType type) {
        switch (type) {
            case VROEyeType::Left:
                return "Left";
            case VROEyeType::Right:
                return "Right";
            default:
                return "Monocular";
        }
    }
    
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
