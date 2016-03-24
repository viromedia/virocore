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

class VROGeometry;
class VROMaterial;
class VROGeometrySubstrate;
class VROMaterialSubstrate;
class VROTextureSubstrate;
class VROData;

enum class VROEyeType;
enum class VROTextureType;
enum class VROTextureFormat;

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
    virtual VROTextureSubstrate *newTextureSubstrate(VROTextureType type, VROTextureFormat format, std::shared_ptr<VROData> data,
                                                     int width, int height) const = 0;
    
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
    std::vector<std::weak_ptr<VROFrameListener>> _frameListeners;
    
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
