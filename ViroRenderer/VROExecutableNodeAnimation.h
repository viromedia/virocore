//
//  VRONodeKeyframeAnimation.h
//  ViroRenderer
//
//  Created by Raj Advani on 7/19/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRONodeKeyframeAnimation_h
#define VRONodeKeyframeAnimation_h

#include "VROExecutableAnimation.h"

/*
 Node animations are executable animations that are fixed to operate on
 a specific node. They essentially wrap a VROExecutableAnimation, but pass
 in their fixed node to the execute(...) method.
 */
class VROExecutableNodeAnimation : public VROExecutableAnimation {
public:
    
    VROExecutableNodeAnimation(std::shared_ptr<VRONode> node, std::shared_ptr<VROExecutableAnimation> executableAnimation) :
        _node(node),
        _executableAnimation(executableAnimation) {
    }
    virtual ~VROExecutableNodeAnimation() {
        
    }
    
    std::shared_ptr<VROExecutableAnimation> copy() {
        std::shared_ptr<VRONode> node = _node.lock();
        if (node) {
            std::shared_ptr<VROExecutableAnimation> inner = _executableAnimation->copy();
            return std::make_shared<VROExecutableNodeAnimation>(node, inner);
        }
        else {
            return {};
        }
    }

    void execute(std::shared_ptr<VRONode> ignoredNode, std::function<void()> onFinished) {
        std::shared_ptr<VRONode> node = _node.lock();
        if (node) {
            _executableAnimation->execute(node, onFinished);
        }
    }
    
    void pause() {
        _executableAnimation->pause();
    }
    void resume() {
        _executableAnimation->resume();
    }
    void terminate(bool jumpToEnd) {
        _executableAnimation->terminate(jumpToEnd);
    }
    void preload() {
        _executableAnimation->preload();
    }

    void setDuration(float durationSeconds) {
        _executableAnimation->setDuration(durationSeconds);
    }
    float getDuration() const {
        return _executableAnimation->getDuration();
    }

    void setTimeOffset(float timeOffset) {
        _executableAnimation->setTimeOffset(timeOffset);
    }

    void setSpeed(float speed) {
        _executableAnimation->setSpeed(speed);
    }

    float getTimeOffset() const {
        return _executableAnimation->getTimeOffset();
    }

    std::string toString() const {
        return _executableAnimation->toString();
    }
    
    std::shared_ptr<VROExecutableAnimation> getInnerAnimation() const {
        return _executableAnimation;
    }
    
private:
    
    std::weak_ptr<VRONode> _node;
    std::shared_ptr<VROExecutableAnimation> _executableAnimation;
    
};

#endif /* VRONodeKeyframeAnimation_h */
