//
//  VRORenderContext.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VRORenderContext_h
#define VRORenderContext_h

#include <stdio.h>
#include <UIKit/UIKit.h>
#include <vector>
#include <memory>
#include "VROFrameListener.h"
#include "VROMatrix4f.h"
#include "VROVector3f.h"
#include "VROQuaternion.h"
#include "VROCamera.h"

enum class VROEyeType;

/*
 Holds data specific to the current frame. Includes things like transformation
 matrices. There is nothing driver or device specific contained here.
 */
class VRORenderContext {
    
public:
    
    VRORenderContext() :
        _frame(0) {
        
    }
    
    int getFrame() const {
        return _frame;
    }
    void setFrame(int frame) {
        _frame = frame;
    }
    
    void setEyeType(VROEyeType eye) {
        _eye = eye;
    }
    VROEyeType getEyeType() const {
        return _eye;
    }
    
    void setProjectionMatrix(VROMatrix4f projectionMatrix) {
        _projectionMatrix = projectionMatrix;
    }
    void setViewMatrix(VROMatrix4f viewMatrix) {
        _viewMatrix = viewMatrix;
    }
    void setMonocularViewMatrix(VROMatrix4f monocularViewMatrix) {
        _monocularViewMatrix = monocularViewMatrix;
    }
    void setHUDViewMatrix(VROMatrix4f hudViewMatrix) {
        _hudViewMatrix = hudViewMatrix;
    }
    void setCamera(VROCamera camera) {
        _camera = camera;
    }
    
    VROMatrix4f getProjectionMatrix() const {
        return _projectionMatrix;
    }
    VROMatrix4f getViewMatrix() const {
        return _viewMatrix;
    }
    VROMatrix4f getMonocularViewMatrix() const {
        return _monocularViewMatrix;
    }
    VROMatrix4f getHUDViewMatrix() const {
        return _hudViewMatrix;
    }
    
    const VROCamera &getCamera() const {
        return _camera;
    }
    
private:
    
    int _frame;
    VROEyeType _eye;
    
    /*
     The standard view and projection matrices. The view matrix is specific for
     the eye currently being rendered (it includes the stereo translation).
     */
    VROMatrix4f _projectionMatrix;
    VROMatrix4f _viewMatrix;
    
    /*
     The view matrix for non-stereo rendered objects.
     */
    VROMatrix4f _monocularViewMatrix;
    
    /*
     The view matrix for objects rendered on the HUD. This is a stereo view matrix:
     it essentially removes head rotation and the camera, but retains the interlens
     distance translation.
     */
    VROMatrix4f _hudViewMatrix;
    
    /*
     The camera used for this frame.
     */
    VROCamera _camera;
    
};

#endif /* VRORenderContext_h */
