//
//  VROKeyframeAnimation.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 7/19/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROKeyframeAnimation.h"
#include "VROTransaction.h"
#include "VROLog.h"
#include "VROAnimationVector3f.h"
#include "VROAnimationQuaternion.h"
#include "VROShaderModifier.h"
#include "VRONode.h"
#include <sstream>
#include <map>

std::shared_ptr<VROExecutableAnimation> VROKeyframeAnimation::copy() {
    pabort("Keyframe animations may not be copied");
}

void VROKeyframeAnimation::execute(std::shared_ptr<VRONode> node, std::function<void()> onFinished) {
    std::weak_ptr<VROKeyframeAnimation> shared_w = shared_from_this();
    
    /*
     Build the key frame animation data.
     */
    std::vector<float> keyTimes;
    std::vector<VROVector3f> translationValues;
    std::vector<VROVector3f> scaleValues;
    std::vector<VROQuaternion> rotationValues;
    
    /*
     Flatten out the data structure so we can pass the keyframes to our
     animation constructors.
     */
    for (std::unique_ptr<VROKeyframeAnimationFrame> &frame : _frames) {
        keyTimes.push_back(frame->time);
        if (_hasTranslation) {
            translationValues.push_back(frame->translation);
        }
        if (_hasScale) {
            scaleValues.push_back(frame->scale);
        }
        if (_hasRotation) {
            rotationValues.push_back(frame->rotation);
        }
    }
    
    VROTransaction::begin();
    VROTransaction::setAnimationDuration(_duration / 1000);
    VROTransaction::setTimingFunction(VROTimingFunctionType::Linear);
    
    if (_hasTranslation) {
        std::shared_ptr<VROAnimation> animation = std::make_shared<VROAnimationVector3f>([shared_w](VROAnimatable *const animatable, VROVector3f v) {
            std::shared_ptr<VROKeyframeAnimation> shared = shared_w.lock();
            if (!shared) {
                return;
            }
            ((VRONode *)animatable)->setPosition(v);
        }, keyTimes, translationValues);
        
        node->animate(animation);
    }
    
    if (_hasRotation) {
        std::shared_ptr<VROAnimation> animation = std::make_shared<VROAnimationQuaternion>([shared_w](VROAnimatable *const animatable, VROQuaternion q) {
            std::shared_ptr<VROKeyframeAnimation> shared = shared_w.lock();
            if (!shared) {
                return;
            }
            ((VRONode *)animatable)->setRotation(q);
        }, keyTimes, rotationValues);
        
        node->animate(animation);
    }
    
    if (_hasScale) {
        std::shared_ptr<VROAnimation> animation = std::make_shared<VROAnimationVector3f>([shared_w](VROAnimatable *const animatable, VROVector3f v) {
            std::shared_ptr<VROKeyframeAnimation> shared = shared_w.lock();
            if (!shared) {
                return;
            }
            ((VRONode *)animatable)->setScale(v);
        }, keyTimes, scaleValues);
        
        node->animate(animation);
    }
    
    std::weak_ptr<VROKeyframeAnimation> weakSelf = shared_from_this();
    VROTransaction::setFinishCallback([weakSelf, onFinished](bool terminate) {
        std::shared_ptr<VROKeyframeAnimation> keyframeAnim = weakSelf.lock();
        if (keyframeAnim) {
            keyframeAnim->_transaction.reset();
        }
        onFinished();
    });
    
    _transaction = VROTransaction::commit();
}

void VROKeyframeAnimation::pause() {
    if (_transaction) {
        VROTransaction::pause(_transaction);
    }
}

void VROKeyframeAnimation::resume() {
    if (_transaction) {
        VROTransaction::resume(_transaction);
    }
}

void VROKeyframeAnimation::terminate() {
    if (_transaction) {
        VROTransaction::terminate(_transaction);
        _transaction.reset();
    }
}

std::string VROKeyframeAnimation::toString() const {
    std::stringstream ss;
    ss << "[keyframe: " << _name << "]";
    return ss.str();
}
