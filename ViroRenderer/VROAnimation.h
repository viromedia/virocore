//
//  VROAnimation.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/22/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROAnimation_h
#define VROAnimation_h

#include <stdio.h>
#include <memory>

class VROAnimation {
    
public:
    
    /*
     Retrieve the active (most deeply nested) uncommitted VROAnimation. Returns 
     nullptr if there is no active VROAnimation.
     */
    static std::shared_ptr<VROAnimation> get();
    
    /*
     Begins a new VROAnimation unless there already is an active animation.
     */
    static void beginImplicitAnimation();
    
    /*
     Update the T values for all committed animations.
     */
    static void updateT();
    
    /*
     Begin a new VROAnimation on this thread, and make this the active animation.
     */
    static void begin();
    
    /*
     Commit the active VROAnimation.
     */
    static void commit();
    
    /*
     Commit all VROAnimations.
     */
    static void commitAll();
    
    /*
     Set the animation duration for the active animation, in seconds.
     */
    static void setAnimationDuration(float durationSeconds);
    static float getAnimationDuration();
    
    VROAnimation();
    ~VROAnimation() {}
    
    /*
     Get the T value (between 0 and 1) representing current progress through this
     animation.
     */
    double getT() {
        return _t;
    }
    
private:
    
    double _t;
    
    double _durationSeconds;
    double _startTimeSeconds;
    
};

#endif /* VROAnimation_h */
