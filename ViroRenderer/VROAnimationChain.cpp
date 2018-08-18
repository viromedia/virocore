//
//  VROAnimationChain.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/28/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROAnimationChain.h"
#include "VROTransaction.h"
#include <sstream>
#include "VROLog.h"

std::shared_ptr<VROExecutableAnimation> VROAnimationChain::copy() {
    std::vector<std::shared_ptr<VROExecutableAnimation>> animations;
    for (std::shared_ptr<VROExecutableAnimation> &animation : _animations) {
        animations.push_back(animation->copy());
    }
    
    return std::make_shared<VROAnimationChain>(animations, _execution);
}

void VROAnimationChain::execute(std::shared_ptr<VRONode> node,
                                std::function<void()> onFinished) {
    
    _numComplete = 0;
    if (_execution == VROAnimationChainExecution::Serial) {
        executeSerial(node, 0, onFinished);
    }
    else {
        executeParallel(node, onFinished);
    }
}

void VROAnimationChain::executeSerial(std::shared_ptr<VRONode> node, int animationIndex,
                                      std::function<void()> onFinished) {
    size_t numAnimations = _animations.size();
    
    std::weak_ptr<VROAnimationChain> weakSelf = shared_from_this();
    std::weak_ptr<VRONode> node_w = node;
    std::shared_ptr<VROExecutableAnimation> &animation = _animations[animationIndex];
    
    std::function<void()> finishCallback = [node_w, weakSelf, animationIndex, numAnimations, onFinished](){
        // Move to next group if chain isn't finished
        if (animationIndex < numAnimations - 1) {
            std::shared_ptr<VROAnimationChain> myself = weakSelf.lock();
            std::shared_ptr<VRONode> node_s = node_w.lock();
            if (myself && node_s) {
                myself->executeSerial(node_s, animationIndex + 1, onFinished);
            }
        }
        else {
            if (onFinished) {
                onFinished();
            }
        }
    };
    
    animation->execute(node, finishCallback);
}

void VROAnimationChain::executeParallel(std::shared_ptr<VRONode> node,
                                        std::function<void()> onFinished) {
    
    size_t numAnimations = _animations.size();
    std::weak_ptr<VROAnimationChain> weakSelf = shared_from_this();
    
    for (std::shared_ptr<VROExecutableAnimation> animation : _animations) {
        std::function<void()> finishCallback = [this, weakSelf, numAnimations, onFinished](){
            // If all groups are complete, execute the onFinished callback
            ++_numComplete;
            
            if (_numComplete == numAnimations) {
                if (onFinished) {
                    onFinished();
                }
            }
        };
        
        animation->execute(node, finishCallback);
    }
}

void VROAnimationChain::resume() {
    for (std::shared_ptr<VROExecutableAnimation> animation : _animations) {
        animation->resume();
    }
}

void VROAnimationChain::pause() {
    for (std::shared_ptr<VROExecutableAnimation> animation : _animations) {
        animation->pause();
    }
}

void VROAnimationChain::terminate(bool jumpToEnd) {
    for (std::shared_ptr<VROExecutableAnimation> animation : _animations) {
        animation->terminate(jumpToEnd);
    }
}

void VROAnimationChain::preload() {
    for (std::shared_ptr<VROExecutableAnimation> animation : _animations) {
        animation->preload();
    }
}

void VROAnimationChain::setDuration(float durationSeconds) {
    for (std::shared_ptr<VROExecutableAnimation> animation : _animations) {
        animation->setDuration(durationSeconds);
    }
}

float VROAnimationChain::getDuration() const {
    float maxDuration = 0;
    for (std::shared_ptr<VROExecutableAnimation> animation : _animations) {
        maxDuration = fmax(maxDuration, animation->getDuration());
    }
    return maxDuration;
}

void VROAnimationChain::addAnimation(std::shared_ptr<VROExecutableAnimation> animation) {
    _animations.push_back(animation);
}

std::string VROAnimationChain::toString() const {
    std::stringstream ss;
    if (_execution == VROAnimationChainExecution::Serial) {
        ss << "[execution: serial, chain [";
    }
    else {
        ss << "[execution: parallel, chain [";
    }
    
    for (std::shared_ptr<VROExecutableAnimation> animation : _animations) {
        ss << " " << animation->toString();
    }
    ss << " ]";
    return ss.str();
}
