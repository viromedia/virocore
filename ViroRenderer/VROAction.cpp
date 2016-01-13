//
//  VROAction.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/12/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROAction.h"
#include "VROTransaction.h"

std::shared_ptr<VROAction> VROAction::perpetualPerFrameAction(std::function<void()> action) {
    std::shared_ptr<VROAction> vAction = std::make_shared<VROAction>(VROActionType::PerFrame,
                                                                     VROActionDurationType::Count,
                                                                     action);
    vAction->_repeatCount = VROActionRepeatForever;
    
    return vAction;
}

std::shared_ptr<VROAction> VROAction::repeatedPerFrameActionFrames(std::function<void()> action, int repeatCount) {
    std::shared_ptr<VROAction> vAction = std::make_shared<VROAction>(VROActionType::PerFrame,
                                                                     VROActionDurationType::Count,
                                                                     action);
    vAction->_repeatCount = repeatCount;
    
    return vAction;
}

std::shared_ptr<VROAction> VROAction::repeatedPerFrameActionSeconds(std::function<void()> action, float duration) {
    std::shared_ptr<VROAction> vAction = std::make_shared<VROAction>(VROActionType::PerFrame,
                                                                     VROActionDurationType::Seconds,
                                                                     action);
    vAction->_repeatCount = VROActionRepeatForever;
    vAction->_duration = duration;
    
    return vAction;
}

std::shared_ptr<VROAction> VROAction::perpetualAnimatedAction(std::function<void()> action, float duration) {
    std::shared_ptr<VROAction> vAction = std::make_shared<VROAction>(VROActionType::Animated,
                                                                     VROActionDurationType::Count,
                                                                     action);
    vAction->_duration = duration;
    vAction->_repeatCount = VROActionRepeatForever;
    
    return vAction;
}

std::shared_ptr<VROAction> VROAction::repeatedAnimatedAction(std::function<void()> action, float duration, int repeatCount) {
    std::shared_ptr<VROAction> vAction = std::make_shared<VROAction>(VROActionType::Animated,
                                                                     VROActionDurationType::Count,
                                                                     action);
    vAction->_duration = duration;
    vAction->_repeatCount = repeatCount;
    
    return vAction;
}

void VROAction::execute() {
    _action();
    
    if (!_executed) {
        _startTime = VROTimeCurrentSeconds();
        _executed = true;
    }
    
    if (_durationType == VROActionDurationType::Count && _repeatCount != VROActionRepeatForever && _repeatCount > 0) {
        --_repeatCount;
    }
}
