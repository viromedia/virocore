//
//  VROAnimationMatrix4f.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROAnimationMatrix4f_h
#define VROAnimationMatrix4f_h

#include "VROMatrix4f.h"
#include "VROAnimation.h"
#include "VROAnimatable.h"
#include "VROMath.h"

class VROAnimationMatrix4f : public VROAnimation {
    
public:
    
    VROAnimationMatrix4f(std::function<void(VROAnimatable *const, VROMatrix4f)> method,
                         VROMatrix4f start,
                         VROMatrix4f end) :
        VROAnimation(),
        _keyTimes({ 0, 1 }),
        _keyValues({ start, end }),
        _method(method)
    {}
    
    VROAnimationMatrix4f(std::function<void(VROAnimatable *const, VROMatrix4f)> method,
                         VROMatrix4f start,
                         VROMatrix4f end,
                         std::function<void(VROAnimatable *const)> finishCallback) :
        VROAnimation(finishCallback),
        _keyTimes({ 0, 1 }),
        _keyValues({ start, end }),
        _method(method)
    {}
    
    VROAnimationMatrix4f(std::function<void(VROAnimatable *const, VROMatrix4f)> method,
                         std::vector<float> keyTimes,
                         std::vector<VROMatrix4f> keyValues) :
        VROAnimation(),
        _keyTimes(keyTimes),
        _keyValues(keyValues),
        _method(method)
    {}
    
    VROAnimationMatrix4f(std::function<void(VROAnimatable *const, VROMatrix4f)> method,
                         std::vector<float> keyTimes,
                         std::vector<VROMatrix4f> keyValues,
                         std::function<void(VROAnimatable *const)> finishCallback) :
        VROAnimation(finishCallback),
        _keyTimes(keyTimes),
        _keyValues(keyValues),
        _method(method)
    {}
    
    void processAnimationFrame(float t) {
        VROMatrix4f value = VROMathInterpolateKeyFrameMatrix4f(t, _keyTimes, _keyValues);
        
        std::shared_ptr<VROAnimatable> animatable = _animatable.lock();
        if (animatable) {
            _method(animatable.get(), value);
        }
    }
    
    void finish() {
        std::shared_ptr<VROAnimatable> animatable = _animatable.lock();
        if (animatable) {
            _method(animatable.get(), _keyValues.back());
        }
    }
    
private:
    
    std::vector<float> _keyTimes;
    std::vector<VROMatrix4f> _keyValues;
    std::function<void(VROAnimatable *const, VROMatrix4f)> _method;
    
};

#endif /* VROAnimationMatrix4f_h */
