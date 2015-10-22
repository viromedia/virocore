//
//  VROAnimation.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/22/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROAnimation.h"
#include "VROTime.h"
#include "VROLog.h"
#include <stack>
#include <vector>

#pragma mark - Animation Management

static std::stack<std::shared_ptr<VROAnimation>> openAnimations;
static std::vector<std::shared_ptr<VROAnimation>> committedAnimations;

std::shared_ptr<VROAnimation> VROAnimation::get() {
    if (openAnimations.empty()) {
        return {};
    }
    else {
        return openAnimations.top();
    }
}

void VROAnimation::beginImplicitAnimation() {
    if (openAnimations.empty()) {
        begin();
    }
}

void VROAnimation::begin() {
    std::shared_ptr<VROAnimation> animation = std::shared_ptr<VROAnimation>(new VROAnimation());
    openAnimations.push(animation);
}

void VROAnimation::commit() {
    std::shared_ptr<VROAnimation> animation = get();
    if (!animation) {
        pabort();
    }

    animation->_t = 0;
    animation->_startTimeSeconds = VROTimeCurrentSeconds();
    
    openAnimations.pop();
    committedAnimations.push_back(animation);
}

void VROAnimation::commitAll() {
    while (!openAnimations.empty()) {
        commit();
    }
}

void VROAnimation::updateT() {
    double time = VROTimeCurrentSeconds();
    
    std::vector<std::shared_ptr<VROAnimation>>::iterator it;
    for (it = committedAnimations.begin(); it != committedAnimations.end(); ++it) {
        std::shared_ptr<VROAnimation> animation = *it;
        animation->_t = (time - animation->_startTimeSeconds) / animation->_durationSeconds;
     
        // Remove the animation when it's complete
        if (animation->_t > 1.0) {
            animation->_t = 1.0;
            
            committedAnimations.erase(it);
            --it;
        }
    }
}

#pragma mark - Animation Class

VROAnimation::VROAnimation() :
    _t(0),
    _durationSeconds(0.2),
    _startTimeSeconds(0)
{}

void VROAnimation::setAnimationDuration(float durationSeconds) {
    std::shared_ptr<VROAnimation> animation = get();
    if (!animation) {
        pabort();
    }
    
    animation->_durationSeconds = durationSeconds;
}

float VROAnimation::getAnimationDuration() {
    std::shared_ptr<VROAnimation> animation = get();
    if (!animation) {
        pabort();
    }

    return animation->_durationSeconds;
}
