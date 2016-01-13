//
//  VROTransaction.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/22/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROTransaction_h
#define VROTransaction_h

#include <stdio.h>
#include <vector>
#include <memory>
#include <functional>
#include "VROAnimation.h"

class VROTransaction {
    
public:
    
    /*
     Retrieve the active (most deeply nested) uncommitted VROTransaction. Returns 
     nullptr if there is no active VROTransaction.
     */
    static std::shared_ptr<VROTransaction> get();
    
    /*
     Return true if there exists an uncommitted VROTransaction with duration greater
     than zero. If false, then animations are considered disabled.
     */
    static bool isActive();
    
    /*
     Begins a new VROTransaction unless there already is an active animation transaction.
     */
    static void beginImplicitAnimation();
    
    /*
     Update the T values for all committed animation transactions.
     */
    static void update();
    
    /*
     Begin a new VROTransaction on this thread, and make this the active animation
     transaction.
     */
    static void begin();
    
    /*
     Set a callback to invoke when the active transaction completes (after duration
     seconds).
     */
    static void setFinishCallback(std::function<void()> finishCallback);
    
    /*
     Commit the active VROTransaction.
     */
    static void commit();
    
    /*
     Commit all VROTransactions.
     */
    static void commitAll();
    
    /*
     Set the animation duration for the active animation transaction, in seconds.
     */
    static void setAnimationDuration(float durationSeconds);
    static float getAnimationDuration();
    
    VROTransaction();
    ~VROTransaction() {}
    
    /*
     Get the T value (between 0 and 1) representing current progress through this
     animation.
     */
    double getT() {
        return _t;
    }
    
    /*
     Return true if this animation has a duration of 0.
     */
    bool isDegenerate() {
        return _durationSeconds <= 0;
    }
    
    /*
     Add a new animation to this transaction.
     */
    void addAnimation(std::shared_ptr<VROAnimation> animation) {
        _animations.push_back(animation);
    }
    
    /*
     Process another frame of all animations in this transaction.
     */
    void processAnimations();
    
    /*
     Invoked when the transaction is finished.
     */
    void onTermination();
    
private:
    
    double _t;
    
    double _durationSeconds;
    double _startTimeSeconds;
    
    std::function<void()> _finishCallback;
    std::vector<std::shared_ptr<VROAnimation>> _animations;
    
};

#endif /* VROTransaction_h */
