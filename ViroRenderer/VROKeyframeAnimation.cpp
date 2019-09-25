//
//  VROKeyframeAnimation.cpp
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

#include "VROKeyframeAnimation.h"
#include "VROTransaction.h"
#include "VROLog.h"
#include "VROAnimationFloat.h"
#include "VROGeometry.h"
#include "VROAnimationVector3f.h"
#include "VROAnimationQuaternion.h"
#include "VROShaderModifier.h"
#include "VRONode.h"
#include "VROMorpher.h"
#include <sstream>
#include <map>

std::shared_ptr<VROExecutableAnimation> VROKeyframeAnimation::copy() {
    std::vector<std::unique_ptr<VROKeyframeAnimationFrame>> frames;
    for (std::unique_ptr<VROKeyframeAnimationFrame> &origFrame : _frames) {
        std::unique_ptr<VROKeyframeAnimationFrame> frame = std::unique_ptr<VROKeyframeAnimationFrame>(new VROKeyframeAnimationFrame());
        frame->time = origFrame->time;
        frame->translation = origFrame->translation;
        frame->scale = origFrame->scale;
        frame->rotation = origFrame->rotation;
        frame->morphWeights = origFrame->morphWeights;

        frames.push_back(std::move(frame));
    }
    std::shared_ptr<VROKeyframeAnimation> animation = std::make_shared<VROKeyframeAnimation>(frames, _duration, _hasTranslation,
                                                                                             _hasRotation, _hasScale, _hasMorphWeights);
    animation->setName(_name);
    animation->setSpeed(_speed);
    animation->setTimeOffset(_timeOffset);
    return animation;
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
    std::map<std::string, std::vector<float>> morphWeightValues;

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
        if (_hasMorphWeights) {
            for (auto target : frame->morphWeights) {
                morphWeightValues[target.first].push_back(target.second);
            }
        }
    }

    // Assume that all key frames have the same number of targets with weighted data.
    if (_hasMorphWeights) {
        for (auto targetWeights : morphWeightValues) {
            passert(_frames.size() == targetWeights.second.size());
        }
    }
    
    VROTransaction::begin();
    VROTransaction::setAnimationDuration(_duration);
    VROTransaction::setAnimationTimeOffset(_timeOffset);
    VROTransaction::setAnimationSpeed(_speed);
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

    if (_hasMorphWeights) {
        // Iterate through each target and its vec of keyframe weights and create
        // a keyframe animation out of them.
        std::map<std::string, std::vector<float>>::iterator it;
        for (it = morphWeightValues.begin(); it != morphWeightValues.end(); it++) {
            std::string morphKey = it->first;
            const std::vector<float> &morphValues = it->second;

            std::shared_ptr<VROAnimation> animation = std::make_shared<VROAnimationFloat>([shared_w, morphKey](VROAnimatable *const animatable, float value) {
                std::shared_ptr<VROKeyframeAnimation> shared = shared_w.lock();
                if (!shared) {
                    return;
                }

                for (auto morpher: ((VROGeometry *)animatable)->getMorphers()) {
                    // Avoid a second costly callback by not triggering
                    // a transaction animation on the VROMorpher itself
                    morpher.second->setWeightForTarget(morphKey, value, false);
                }
            }, keyTimes, morphValues);
            node->getGeometry()->animate(animation);
        }
    }

    std::weak_ptr<VROKeyframeAnimation> weakSelf = shared_from_this();
    VROTransaction::setFinishCallback([weakSelf, onFinished](bool terminate) {
        std::shared_ptr<VROKeyframeAnimation> keyframeAnim = weakSelf.lock();
        if (keyframeAnim) {
            keyframeAnim->_transaction.reset();
        }
        onFinished();
    });
    
    std::shared_ptr<VROTransaction> transaction = VROTransaction::commit();
    transaction->holdExecutableAnimation(shared_from_this());
    
    _transaction = transaction;
}

void VROKeyframeAnimation::pause() {
    std::shared_ptr<VROTransaction> transaction = _transaction.lock();
    if (transaction) {
        VROTransaction::pause(transaction);
    }
}

void VROKeyframeAnimation::resume() {
    std::shared_ptr<VROTransaction> transaction = _transaction.lock();
    if (transaction) {
        VROTransaction::resume(transaction);
    }
}

void VROKeyframeAnimation::terminate(bool jumpToEnd) {
    std::shared_ptr<VROTransaction> transaction = _transaction.lock();
    if (transaction) {
        VROTransaction::terminate(transaction, jumpToEnd);
        _transaction.reset();
    }
}

void VROKeyframeAnimation::setSpeed(float speed) {
    _speed = speed;
    std::shared_ptr<VROTransaction> transaction = _transaction.lock();
    if (transaction) {
        VROTransaction::setAnimationSpeed(transaction, _speed);
    }
}

std::string VROKeyframeAnimation::toString() const {
    std::stringstream ss;
    ss << "[keyframe: " << _name << "]";
    return ss.str();
}
