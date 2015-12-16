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
#include <memory>

class VROTransaction {
    
public:
    
    /*
     Retrieve the active (most deeply nested) uncommitted VROTransaction. Returns 
     nullptr if there is no active VROTransaction.
     */
    static std::shared_ptr<VROTransaction> get();
    
    /*
     Begins a new VROTransaction unless there already is an active animation transaction.
     */
    static void beginImplicitAnimation();
    
    /*
     Update the T values for all committed animation transactions.
     */
    static void updateT();
    
    /*
     Begin a new VROTransaction on this thread, and make this the active animation
     transaction.
     */
    static void begin();
    
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
    
private:
    
    double _t;
    
    double _durationSeconds;
    double _startTimeSeconds;
    
};

#endif /* VROTransaction_h */
