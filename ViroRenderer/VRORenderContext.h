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

class VROGeometry;
class VROMaterial;
class VROGeometrySubstrate;
class VROMaterialSubstrate;
class VROTextureSubstrate;

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
                locked->onFrameWillRender();
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
                locked->onFrameDidRender();
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
    
private:
    
    int _frame;
    std::vector<std::weak_ptr<VROFrameListener>> _frameListeners;
    
};

#endif /* VRORenderContext_h */
