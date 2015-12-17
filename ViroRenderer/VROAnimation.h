//
//  VROAnimation.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROAnimation_h
#define VROAnimation_h

#include <stdio.h>
#include <memory>
#include <functional>

class VROAnimatable;

class VROAnimation {
    
public:
    
    VROAnimation(std::shared_ptr<VROAnimatable> animatable) :
        _animatable(animatable)
    {}
    
    virtual void processAnimationFrame(float t) = 0;
    
protected:
    
    std::weak_ptr<VROAnimatable> _animatable;
    
};

#endif /* VROAnimation_h */
