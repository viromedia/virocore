//
//  VRONodeKeyframeAnimation.h
//  ViroRenderer
//
//  Created by Raj Advani on 7/19/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
