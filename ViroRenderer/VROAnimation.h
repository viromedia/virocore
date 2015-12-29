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

/*
 Represents the animation of a property. Subclasses identify the type
 of the property (e.g. animating a floating point value, animating a
 VROVector3f, etc.).
 */
class VROAnimation {
    
public:
    
    VROAnimation() {}
    
    /*
     Set the animatable. This is typically the object holding the 
     property to be animated. It is stored by a weak_ptr so we can
     ensure it hasn't been deallocated before invoking the animation
     each frame.
     */
    void setAnimatable(std::shared_ptr<VROAnimatable> animatable) {
        _animatable = animatable;
    }
    
    /*
     Move the property to its value corresponding to t [0, 1].
     */
    virtual void processAnimationFrame(float t) = 0;
    
    /*
     Immediately finish this animation by moving its value to the
     end state.
     */
    virtual void finish() = 0;
    
protected:
    
    std::weak_ptr<VROAnimatable> _animatable;
    
};

#endif /* VROAnimation_h */
