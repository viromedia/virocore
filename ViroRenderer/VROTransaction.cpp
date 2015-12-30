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

void VROTransaction::commitAll() {
    while (!openAnimations.empty()) {
        commit();
    }
}

void VROTransaction::update() {
    double time = VROTimeCurrentSeconds();
    
    std::vector<std::shared_ptr<VROTransaction>>::iterator it;
    for (it = committedAnimations.begin(); it != committedAnimations.end(); ++it) {
        std::shared_ptr<VROTransaction> animation = *it;
        animation->_t = (time - animation->_startTimeSeconds) / animation->_durationSeconds;
        
        animation->processAnimations();
     
        // Remove the animation when it's complete
        if (animation->_t > 1.0) {
            animation->_t = 1.0;
            
            committedAnimations.erase(it);
            --it;
        }
    }
}

#pragma mark - Transaction Class

VROTransaction::VROTransaction() :
    _t(0),
    _durationSeconds(0),
    _startTimeSeconds(0)
{}

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

void VROTransaction::processAnimations() {
    for (std::shared_ptr<VROAnimation> animation : _animations) {
        animation->processAnimationFrame(_t);
    }
}
