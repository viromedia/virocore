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
    std::shared_ptr<VROExecutableAnimation> &animation = _animations[animationIndex];
    
    std::function<void()> finishCallback = [node, weakSelf, animationIndex, numAnimations, onFinished](){
        // Move to next group if chain isn't finished
        if (animationIndex < numAnimations - 1) {
            std::shared_ptr<VROAnimationChain> myself = weakSelf.lock();
            if (myself) {
                myself->executeSerial(node, animationIndex + 1, onFinished);
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
        std::function<void()> finishCallback = [this, node, weakSelf, numAnimations, onFinished](){
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

void VROAnimationChain::terminate() {
    for (std::shared_ptr<VROExecutableAnimation> animation : _animations) {
        animation->terminate();
    }
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
