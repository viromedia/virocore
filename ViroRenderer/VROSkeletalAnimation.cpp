//
//  VROSkeletalAnimation.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/16/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSkeletalAnimation.h"
#include "VROTransaction.h"
#include "VROLog.h"
#include "VROAnimationMatrix4f.h"
#include "VROSkeleton.h"
#include "VROShaderModifier.h"
#include "VROBone.h"
#include <sstream>
#include <map>

std::shared_ptr<VROExecutableAnimation> VROSkeletalAnimation::copy() {
    std::vector<std::unique_ptr<VROSkeletalAnimationFrame>> frames;
    for (std::unique_ptr<VROSkeletalAnimationFrame> &origFrame : _frames) {
        std::unique_ptr<VROSkeletalAnimationFrame> frame = std::unique_ptr<VROSkeletalAnimationFrame>(new VROSkeletalAnimationFrame());
        frame->time = origFrame->time;
        frame->boneIndices = origFrame->boneIndices;
        frame->boneTransforms = origFrame->boneTransforms;

        frames.push_back(std::move(frame));
    }
    std::shared_ptr<VROSkeletalAnimation> animation = std::make_shared<VROSkeletalAnimation>(_skeleton, frames, _duration);
    animation->setName(_name);
    
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
    VROTransaction::setTimingFunction(VROTimingFunctionType::Linear);
    
    for (auto kv : boneKeyTimes) {
        int boneIndex = kv.first;
        std::shared_ptr<VROBone> bone = _skeleton->getBone(boneIndex);
        
        std::vector<float> &keyTimes = kv.second;
        std::vector<VROMatrix4f> &keyValues = boneKeyValues[boneIndex];
        std::shared_ptr<VROAnimation> animation = std::make_shared<VROAnimationMatrix4f>([shared_w, boneIndex](VROAnimatable *const animatable, VROMatrix4f m) {
            std::shared_ptr<VROSkeletalAnimation> shared = shared_w.lock();
            if (!shared) {
                return;
            }
            ((VROBone *)animatable)->setTransform(m);
        }, keyTimes, keyValues);
        
        bone->animate(animation);
    }
    
    std::weak_ptr<VROSkeletalAnimation> weakSelf = shared_from_this();
    VROTransaction::setFinishCallback([weakSelf, onFinished](bool terminate) {
        std::shared_ptr<VROSkeletalAnimation> skeletal = weakSelf.lock();
        if (skeletal) {
            skeletal->_transaction.reset();
        }
        onFinished();
    });
    
    std::shared_ptr<VROTransaction> transaction = VROTransaction::commit();
    transaction->holdExecutableAnimation(shared_from_this());
    
    _transaction = transaction;
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

std::string VROSkeletalAnimation::toString() const {
    std::stringstream ss;
    ss << "[skeletal: " << _name << "]";
    return ss.str();
}
