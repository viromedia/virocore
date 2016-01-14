//
//  VROTransaction.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/22/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROTransaction.h"
#include "VROTime.h"
#include "VROLog.h"
#include "VROTimingFunctionLinear.h"
#include <stack>
#include <vector>

#pragma mark - Transaction Management

static std::stack<std::shared_ptr<VROTransaction>> openAnimations;
static std::vector<std::shared_ptr<VROTransaction>> committedAnimations;

std::shared_ptr<VROTransaction> VROTransaction::get() {
    if (openAnimations.empty()) {
        return {};
    }
    else {
        return openAnimations.top();
    }
}

bool VROTransaction::isActive() {
    return !openAnimations.empty() && openAnimations.top()->_durationSeconds > 0;
}

void VROTransaction::beginImplicitAnimation() {
    if (openAnimations.empty()) {
        begin();
    }
}

void VROTransaction::begin() {
    std::shared_ptr<VROTransaction> animation = std::shared_ptr<VROTransaction>(new VROTransaction());
    openAnimations.push(animation);
}

void VROTransaction::commit() {
    std::shared_ptr<VROTransaction> animation = get();
    if (!animation) {
        pabort();
    }

    animation->_t = 0;
    animation->_startTimeSeconds = VROTimeCurrentSeconds();
    
    openAnimations.pop();
    committedAnimations.push_back(animation);
}

void VROTransaction::setFinishCallback(std::function<void ()> finishCallback) {
    std::shared_ptr<VROTransaction> animation = get();
    if (!animation) {
        pabort();
    }
    
    animation->_finishCallback = finishCallback;
}

void VROTransaction::setTimingFunction(VROTimingFunctionType timingFunctionType) {
    setTimingFunction(VROTimingFunction::forType(timingFunctionType));
}

void VROTransaction::setTimingFunction(std::unique_ptr<VROTimingFunction> timingFunction) {
    std::shared_ptr<VROTransaction> animation = get();
    if (!animation) {
        pabort();
    }
    
    animation->_timingFunction = std::move(timingFunction);
}

void VROTransaction::commitAll() {
    while (!openAnimations.empty()) {
        commit();
    }
}

void VROTransaction::update() {
    double time = VROTimeCurrentSeconds();
    
    std::vector<std::shared_ptr<VROTransaction>>::iterator it;
    for (it = committedAnimations.begin(); it != committedAnimations.end(); ++it) {
        std::shared_ptr<VROTransaction> transaction = *it;
        
        float t = (time - transaction->_startTimeSeconds) / transaction->_durationSeconds;
        transaction->processAnimations(t);
     
        // Remove the transaction when it's complete
        if (transaction->_t > 1.0) {
            transaction->onTermination();
            committedAnimations.erase(it);
            
            --it;
        }
    }
}

#pragma mark - Transaction Class

VROTransaction::VROTransaction() :
    _t(0),
    _durationSeconds(0),
    _startTimeSeconds(0) {
    
    _timingFunction = std::unique_ptr<VROTimingFunction>(new VROTimingFunctionLinear());
}

void VROTransaction::setAnimationDuration(float durationSeconds) {
    std::shared_ptr<VROTransaction> animation = get();
    if (!animation) {
        pabort();
    }
    
    animation->_durationSeconds = durationSeconds;
}

float VROTransaction::getAnimationDuration() {
    std::shared_ptr<VROTransaction> animation = get();
    if (!animation) {
        pabort();
    }

    return animation->_durationSeconds;
}

void VROTransaction::processAnimations(float t) {
    if (isinf(t)) {
        return;
    }
    _t = t;
    float transformedT = _timingFunction->getT(t);
    
    for (std::shared_ptr<VROAnimation> animation : _animations) {
        animation->processAnimationFrame(transformedT);
    }
}

void VROTransaction::onTermination() {
    _t = 1.0;
    
    for (std::shared_ptr<VROAnimation> animation : _animations) {
        animation->onTermination();
    }
    if (_finishCallback) {
        _finishCallback();
    }
}
