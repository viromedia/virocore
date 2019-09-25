//
//  VROSkeletalAnimation.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/16/17.
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

#include "VROSkeletalAnimation.h"
#include "VROTransaction.h"
#include "VROLog.h"
#include "VROAnimationMatrix4f.h"
#include "VROSkeleton.h"
#include "VROShaderModifier.h"
#include "VROBone.h"
#include "VROSkinner.h"
#include <sstream>
#include <map>

std::shared_ptr<VROExecutableAnimation> VROSkeletalAnimation::copy() {
    std::vector<std::unique_ptr<VROSkeletalAnimationFrame>> frames;
    for (std::unique_ptr<VROSkeletalAnimationFrame> &origFrame : _frames) {
        std::unique_ptr<VROSkeletalAnimationFrame> frame = std::unique_ptr<VROSkeletalAnimationFrame>(new VROSkeletalAnimationFrame());
        frame->time = origFrame->time;
        frame->boneIndices = origFrame->boneIndices;
        frame->boneTransforms = origFrame->boneTransforms;
        frame->boneTransformsLegacy = origFrame->boneTransformsLegacy;

        frames.push_back(std::move(frame));
    }

    std::shared_ptr<VROSkeletalAnimation> animation = std::make_shared<VROSkeletalAnimation>(_skinner, frames, _duration);
    animation->setName(_name);
    animation->setTimeOffset(_timeOffset);
    animation->setSpeed(_speed);
    return animation;
}

void VROSkeletalAnimation::execute(std::shared_ptr<VRONode> node, std::function<void()> onFinished) {
    std::weak_ptr<VROSkeletalAnimation> shared_w = shared_from_this();
    
    /*
     Build the key frame animation data.
     */
    std::map<int, std::vector<float>> boneKeyTimes;
    std::map<int, std::vector<VROMatrix4f>> boneKeyValues;

    for (std::unique_ptr<VROSkeletalAnimationFrame> &frame : _frames) {
        passert (frame->boneIndices.size() == frame->boneTransforms.size());
        
        for (int i = 0; i < frame->boneIndices.size(); i++) {
            int boneIndex = frame->boneIndices[i];
            VROMatrix4f &transform = frame->boneTransforms[i];
            
            boneKeyValues[boneIndex].push_back(transform);
            boneKeyTimes[boneIndex].push_back(frame->time);
        }
    }
    
    VROTransaction::begin();
    VROTransaction::setAnimationDuration(_duration);
    VROTransaction::setAnimationTimeOffset(_timeOffset);
    VROTransaction::setAnimationSpeed(_speed);
    VROTransaction::setTimingFunction(VROTimingFunctionType::Linear);
    
    for (auto kv : boneKeyTimes) {
        int boneIndex = kv.first;
        std::shared_ptr<VROBone> bone = _skinner->getSkeleton()->getBone(boneIndex);
        
        std::vector<float> &keyTimes = kv.second;
        std::vector<VROMatrix4f> &keyValues = boneKeyValues[boneIndex];
        std::shared_ptr<VROAnimation> animation = std::make_shared<VROAnimationMatrix4f>([shared_w, boneIndex](VROAnimatable *const animatable, VROMatrix4f m) {
            std::shared_ptr<VROSkeletalAnimation> shared = shared_w.lock();
            if (!shared) {
                return;
            }
            VROBone *bone = ((VROBone *)animatable);
            bone->setTransform(m, bone->getTransformType());
        }, keyTimes, keyValues);
        
        bone->animate(animation);
    }
    
    VROTransaction::setFinishCallback([shared_w, onFinished](bool terminate) {
        std::shared_ptr<VROSkeletalAnimation> shared = shared_w.lock();
        if (shared) {
            shared->_transaction.reset();
        }
        onFinished();
    });
    
    std::shared_ptr<VROTransaction> transaction = VROTransaction::commit();
    transaction->holdExecutableAnimation(shared_from_this());
    
    _transaction = transaction;
}

void VROSkeletalAnimation::setSpeed(float speed) {
    _speed = speed;
    std::shared_ptr<VROTransaction> transaction = _transaction.lock();
    if (transaction) {
        VROTransaction::setAnimationSpeed(transaction, speed);
    }
}

void VROSkeletalAnimation::pause() {
    std::shared_ptr<VROTransaction> transaction = _transaction.lock();
    if (transaction) {
        VROTransaction::pause(transaction);
    }
}

void VROSkeletalAnimation::resume() {
    std::shared_ptr<VROTransaction> transaction = _transaction.lock();
    if (transaction) {
        VROTransaction::resume(transaction);
    }
}

void VROSkeletalAnimation::terminate(bool jumpToEnd) {
    std::shared_ptr<VROTransaction> transaction = _transaction.lock();
    if (transaction) {
        VROTransaction::terminate(transaction, jumpToEnd);
        _transaction.reset();
    }
}

void VROSkeletalAnimation::setDuration(float durationSeconds) {
    _duration = durationSeconds;
}

float VROSkeletalAnimation::getDuration() const {
    return _duration;
}

std::string VROSkeletalAnimation::toString() const {
    std::stringstream ss;
    ss << "[skeletal: " << _name << "]";
    return ss.str();
}
