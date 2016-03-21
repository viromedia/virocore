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

class VROGeometry;
class VROMaterial;
class VROGeometrySubstrate;
class VROMaterialSubstrate;
class VROTextureSubstrate;

enum class VROEyeType;
enum class VROTextureType;

/*
 Contains the Metal or OpenGL context objects required to render a layer.
 In Metal, these are things like the render pass descriptor, which defines
 the target for rendering.
 */
class VRORenderContext {
    
public:
    
    VRORenderContext() :
        _frame(0) {
        
    }
    
    virtual VROGeometrySubstrate *newGeometrySubstrate(const VROGeometry &geometry) const = 0;
    virtual VROMaterialSubstrate *newMaterialSubstrate(VROMaterial &material) const = 0;
    virtual VROTextureSubstrate *newTextureSubstrate(VROTextureType type, std::vector<UIImage *> &images) const = 0;
    
    void addFrameListener(std::shared_ptr<VROFrameListener> listener) {
        _frameListeners.push_back(listener);
    }
    void removeFrameListener(std::shared_ptr<VROFrameListener> listener) {
        _frameListeners.erase(
                              std::remove_if(_frameListeners.begin(), _frameListeners.end(),
                                             [this, listener](std::weak_ptr<VROFrameListener> l) {
                                                 std::shared_ptr<VROFrameListener> locked = l.lock();
                                                 return locked && locked == listener;
                                             }), _frameListeners.end());
    }
    
    void notifyFrameStart() {
        auto it = _frameListeners.begin();
        
        while (it != _frameListeners.end()) {
            std::weak_ptr<VROFrameListener> listener = *it;
            std::shared_ptr<VROFrameListener> locked = listener.lock();
            
            if (locked) {
                locked->onFrameWillRender(*this);
                ++it;
            }
            else {
                it = _frameListeners.erase(it);
            }
        }
    }
    void notifyFrameEnd() {
        auto it = _frameListeners.begin();
        
        while (it != _frameListeners.end()) {
            std::weak_ptr<VROFrameListener> listener = *it;
            std::shared_ptr<VROFrameListener> locked = listener.lock();
            
            if (locked) {
                locked->onFrameDidRender(*this);
                ++it;
            }
            else {
                it = _frameListeners.erase(it);
            }
        }
    }
    
    int getFrame() const {
        return _frame;
    }
    void incFrame() {
        ++_frame;
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
    void setCameraForward(VROVector3f cameraForward) {
        _cameraForward = cameraForward;
    }
    void setCameraQuaternion(VROQuaternion cameraQuaternion) {
        _cameraQuaternion = cameraQuaternion;
    }
    
    VROMatrix4f getProjectionMatrix() const {
        return _projectionMatrix;
    }
    VROMatrix4f getViewMatrix() const {
        return _viewMatrix;
    }
    VROVector3f getCameraForward() const {
        return _cameraForward;
    }
    VROQuaternion getCameraQuaternion() const {
        return _cameraQuaternion;
    }
    
private:
    
    int _frame;
    VROEyeType _eye;
    std::vector<std::weak_ptr<VROFrameListener>> _frameListeners;
    
    /*
     The standard view and projection matrices.
     */
    VROMatrix4f _projectionMatrix;
    VROMatrix4f _viewMatrix;
    
    /*
     The camera forward vector, and the camera quaternion. The quaternion represents
     the rotation from (0, 0, -1) required to achieve the camera's current
     orientation.
     */
    VROVector3f _cameraForward;
    VROQuaternion _cameraQuaternion;
    
};

#endif /* VRORenderContext_h */
