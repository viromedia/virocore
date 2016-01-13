//
//  VROAction.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/12/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROAction_h
#define VROAction_h

#include <stdio.h>
#include <functional>
#include "VROTime.h"

class VRONode;

static const int VROActionRepeatForever = -1;

enum class VROActionType {
    PerFrame,
    Animated
};

enum class VROActionDurationType {
    Count,
    Seconds
};

/*
 Actions are callbacks that can be set to run on VRONodes. They can
 be used to animate properties of nodes, or to set repeating actions.
 */
class VROAction {
    
public:
    
    static std::shared_ptr<VROAction> perpetualPerFrameAction(std::function<void()> action);
    static std::shared_ptr<VROAction> repeatedPerFrameActionFrames(std::function<void()> action, int repeatCount);
    static std::shared_ptr<VROAction> repeatedPerFrameActionSeconds(std::function<void()> action, float duration);
    
    static std::shared_ptr<VROAction> perpetualAnimatedAction(std::function<void()> action, float duration);
    static std::shared_ptr<VROAction> repeatedAnimatedAction(std::function<void()> action, float duration, int repeatCount);
    
    VROActionType getType() const {
        return _type;
    }
    float getDuration() const {
        return _duration;
    }
    bool shouldRepeat() const {
        if (_durationType == VROActionDurationType::Count) {
            return _repeatCount > 0 || _repeatCount == VROActionRepeatForever;
        }
        else {
            return (VROTimeCurrentSeconds() - _startTime) < _duration;
        }
    }
    
    /*
     Internal: execute the action and decrement repeat count.
     */
    void execute();
    
    VROAction(VROActionType type, VROActionDurationType durationType, std::function<void()> action) :
        _type(type),
        _durationType(durationType),
        _executed(false),
        _action(action)
    {}
    ~VROAction() {}
    
private:
    
    VROActionType _type;
    VROActionDurationType _durationType;
    
    /*
     True after the action has been executed at least once.
     */
    bool _executed;
    
    /*
     Callback for the action itself (i.e. callback which performs
     the action).
     */
    std::function<void()> _action;
    
    /*
     Duration of the action, in seconds. Only valid for animated actions,
     defines the duration of *each* repetition of the animation.
     */
    float _duration;
    
    /*
     The number of times to repeat this action, or the number of seconds to
     repeat the action.
     */
    int _repeatCount;
    float _repeatDuration;
    
    /*
     The time at which the action was first executed.
     */
    float _startTime;
    
};

#endif /* VROAction_h */
