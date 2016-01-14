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
#include "VROTimingFunction.h"

class VRONode;

static const int VROActionRepeatForever = -1;

enum class VROActionType {
    PerFrame,
    Timed,
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
class VROAction : public std::enable_shared_from_this<VROAction> {
    
public:
    
    /*
     For per-frame actions, the given function will be run once per frame. These will
     run either perpetually, or for the given number of frames or actions.
     */
    static std::shared_ptr<VROAction> perpetualPerFrameAction(std::function<void()> action);
    static std::shared_ptr<VROAction> repeatedPerFrameActionFrames(std::function<void()> action, int repeatCount);
    static std::shared_ptr<VROAction> repeatedPerFrameActionSeconds(std::function<void()> action, float duration);
    
    /*
     For timed actions, the given function will be run each frame until the given number of
     seconds. The function takes a variable t that runs from [0,1] over the course of the duration,
     transformed by the provided timing function.
     */
    static std::shared_ptr<VROAction> timedAction(std::function<void(float)> action,
                                                  VROTimingFunctionType timingFunction,
                                                  float duration);
    
    /*
     For animated actions, the given function will be run once, within a VROTransaction configured
     to use the given timing function and duration. These are merely helper functions that sit over
     the VROTransaction animation framework.
     */
    static std::shared_ptr<VROAction> perpetualAnimatedAction(std::function<void()> action,
                                                              VROTimingFunctionType timingFunction,
                                                              float duration);
    static std::shared_ptr<VROAction> repeatedAnimatedAction(std::function<void()> action,
                                                             VROTimingFunctionType timingFunction,
                                                             float duration, int repeatCount);
    
    VROAction(VROActionType type, VROActionDurationType durationType) :
        _type(type),
        _durationType(durationType),
        _executed(false)
    {}
    virtual ~VROAction() {}
    
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
    virtual void execute(VRONode *node);
    
protected:
    
    VROActionType _type;
    VROActionDurationType _durationType;
    
    /*
     True after the action has been executed at least once.
     */
    bool _executed;
    
    /*
     Duration of the action, in seconds. Only valid for animated actions,
     defines the duration of *each* repetition of the animation.
     */
    float _duration;
    
    /*
     The number of frames to repeat this action, or the number of seconds to
     repeat the action.
     */
    int _repeatCount;
    float _repeatDuration;
    
    /*
     The time at which the action was first executed.
     */
    float _startTime;
    
};

class VROActionPerFrame : public VROAction {
public:
    
    VROActionPerFrame(std::function<void()> action, VROActionDurationType durationType) :
    VROAction(VROActionType::PerFrame, durationType),
        _action(action)
    {}
    virtual ~VROActionPerFrame() {}
    virtual void execute(VRONode *node);
    
private:
    std::function<void()> _action;
    
};

class VROActionTimed : public VROAction {
public:
    
    VROActionTimed(std::function<void(float)> action, VROTimingFunctionType timingFunctionType) :
        VROAction(VROActionType::Timed, VROActionDurationType::Seconds),
        _action(action),
        _timingFunction(VROTimingFunction::forType(timingFunctionType))
    {}
    virtual ~VROActionTimed() {}
    virtual void execute(VRONode *node);
    
private:
    std::function<void(float)> _action;
    std::unique_ptr<VROTimingFunction> _timingFunction;
};

class VROActionAnimated : public VROAction {
public:
    
    VROActionAnimated(std::function<void()> action, VROTimingFunctionType timingFunctionType) :
        VROAction(VROActionType::Animated, VROActionDurationType::Count),
        _action(action),
        _timingFunctionType(timingFunctionType)
    {}
    virtual ~VROActionAnimated() {}
    virtual void execute(VRONode *node);
    
private:
    std::function<void()> _action;
    VROTimingFunctionType _timingFunctionType;
};



#endif /* VROAction_h */
