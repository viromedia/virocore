//
//  VROAnimationFloat.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROAnimationFloat_h
#define VROAnimationFloat_h

#include "VROAnimation.h"
#include "VROAnimatable.h"

class VROAnimationFloat : public VROAnimation {
    
public:
    
    VROAnimationFloat(std::shared_ptr<VROAnimatable> target,
                      std::function<void(float)> method,
                      float start, float end) :
        VROAnimation(target),
        _start(start),
        _end(end),
        _method(method)
    {}
    
    void processAnimationFrame(float t) {
        float value = _start + (_end - _start) * t;
        
        std::shared_ptr<VROAnimatable> animatable = _animatable.lock();
        if (animatable) {
            _method(value);
        }
    }
    
    void finish() {
        std::shared_ptr<VROAnimatable> animatable = _animatable.lock();
        if (animatable) {
            _method(_end);
        }
    }

private:
    
    float _start, _end;
    std::function<void(float)>  _method;
    
};

#endif /* VROAnimationFloat_h */
