//
//  VROAnimationMatrix4f.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
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
