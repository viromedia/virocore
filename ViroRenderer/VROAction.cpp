//
//  VROAction.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/12/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROAction.h"
#include "VROTransaction.h"
#include "VRONode.h"
#include "VROMath.h"

std::shared_ptr<VROAction> VROAction::perpetualPerFrameAction(std::function<bool(VRONode *const, float)> action) {
    std::shared_ptr<VROAction> vAction = std::make_shared<VROActionPerFrame>(action,
                                                                             VROActionDurationType::Count);
    vAction->_repeatCount = VROActionRepeatForever;
    return vAction;
}

std::shared_ptr<VROAction> VROAction::repeatedPerFrameActionFrames(std::function<bool(VRONode *const, float)> action, int repeatCount) {
    std::shared_ptr<VROAction> vAction = std::make_shared<VROActionPerFrame>(action,
                                                                             VROActionDurationType::Count);
    vAction->_repeatCount = repeatCount;
    return vAction;
}

std::shared_ptr<VROAction> VROAction::repeatedPerFrameActionSeconds(std::function<bool(VRONode *const, float)> action, float duration) {
    std::shared_ptr<VROAction> vAction = std::make_shared<VROActionPerFrame>(action, VROActionDurationType::Seconds);
    vAction->_repeatCount = VROActionRepeatForever;
    vAction->_duration = duration;
    return vAction;
}

std::shared_ptr<VROAction> VROAction::timedAction(std::function<void(VRONode *const, float)> action,
                                                  VROTimingFunctionType timingFunction,
                                                  float duration) {
    
    std::shared_ptr<VROAction> vAction = std::make_shared<VROActionTimed>(action, timingFunction);
    vAction->_repeatCount = VROActionRepeatForever;
    vAction->_duration = duration;
    return vAction;
}

std::shared_ptr<VROAction> VROAction::perpetualAnimatedAction(std::function<void(VRONode *const)> action,
                                                              VROTimingFunctionType timingFunction,
                                                              float duration) {
    std::shared_ptr<VROAction> vAction = std::make_shared<VROActionAnimated>(action, timingFunction);
    vAction->_repeatCount = VROActionRepeatForever;
    vAction->_duration = duration;
    return vAction;
}

std::shared_ptr<VROAction> VROAction::repeatedAnimatedAction(std::function<void(VRONode *const)> action,
                                                             VROTimingFunctionType timingFunction,
                                                             float duration, int repeatCount) {
    std::shared_ptr<VROAction> vAction = std::make_shared<VROActionAnimated>(action, timingFunction);
    vAction->_repeatCount = repeatCount;
    vAction->_duration = duration;
    return vAction;
}

void VROAction::preExecute(VRONode *node) {
    if (!_executed) {
        _startTime = VROTimeCurrentSeconds();
        _executed = true;
    }
}

void VROAction::postExecute(VRONode *node) {
    if (_durationType == VROActionDurationType::Count &&
        _repeatCount != VROActionRepeatForever && _repeatCount > 0) {
        
        --_repeatCount;
    }
}

void VROActionPerFrame::execute(VRONode *node) {
    VROAction::preExecute(node);
    _aborted = !_action(node, _executed ? (VROTimeCurrentSeconds() - _startTime) : 0);
    VROAction::postExecute(node);
}

void VROActionTimed::execute(VRONode *node) {
    VROAction::preExecute(node);
    
    float t = VROMathClamp((VROTimeCurrentSeconds() - _startTime) / _duration, 0.0, 1.0);
    _action(node, _timingFunction->getT(t));
    
    VROAction::postExecute(node);
}

void VROActionAnimated::execute(VRONode *node) {
    VROAction::preExecute(node);
    std::shared_ptr<VROAction> shared = shared_from_this();
    
    VROTransaction::begin();
    VROTransaction::setAnimationDuration(_duration);
    VROTransaction::setTimingFunction(_timingFunctionType);
    VROTransaction::setFinishCallback([shared, node](bool terminate) {
        if (shared->shouldRepeat()) {
            node->runAction(shared);
        }
    });
    
    _action(node);
    VROAction::postExecute(node);
    
    VROTransaction::commit();
}
