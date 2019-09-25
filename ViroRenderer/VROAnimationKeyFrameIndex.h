//
//  VROAnimationKeyFrameIndex.h
//  ViroKit
//
//  Created by Raj Advani on 8/22/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROAnimationKeyFrameIndex_h
#define VROAnimationKeyFrameIndex_h

#include "VROAnimation.h"
#include "VROAnimatable.h"
#include "VROMath.h"

class VROAnimationKeyframeIndex : public VROAnimation {
    
public:
    
    VROAnimationKeyframeIndex(std::function<void(VROAnimatable *const, int)> method,
                              std::vector<float> keyTimes) :
    VROAnimation(),
    _keyTimes(keyTimes),
    _method(method)
    {}
    
    VROAnimationKeyframeIndex(std::function<void(VROAnimatable *const, int)> method,
                              std::vector<float> keyTimes,
                              std::function<void(VROAnimatable *const)> finishCallback) :
    VROAnimation(finishCallback),
    _keyTimes(keyTimes),
    _method(method)
    {}
    
    void processAnimationFrame(float t) {
        int frame = VROMathInterpolateKeyFrameIndex(t, _keyTimes);
        
        std::shared_ptr<VROAnimatable> animatable = _animatable.lock();
        if (animatable) {
            _method(animatable.get(), frame);
        }
    }
    
    void finish() {
        std::shared_ptr<VROAnimatable> animatable = _animatable.lock();
        if (animatable) {
            _method(animatable.get(), (int) _keyTimes.size() - 1);
        }
    }
    
private:
    
    std::vector<float> _keyTimes;
    std::function<void(VROAnimatable *const, int)>  _method;
    
};

#endif /* VROAnimationKeyFrameIndex_h */
