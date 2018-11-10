//
//  VROKeyframeAnimation.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 7/19/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROKeyframeAnimation_h
#define VROKeyframeAnimation_h

#include <memory>
#include <vector>
#include <map>
#include "VROVector3f.h"
#include "VROQuaternion.h"
#include "VROExecutableAnimation.h"

class VROShaderModifier;

/*
 Single frame of a keyframe animation.
 */
struct VROKeyframeAnimationFrame {
    
    /*
     Start time of this frame. Defined between [0, 1], where 0
     is the start of the animation and 1.0 is the end.
     */
    float time;
    
    /*
     The values to assign for translation, rotation, and scale. A given 
     keyframe does not have to include all of these.
     */
    VROVector3f translation;
    VROVector3f scale;
    VROQuaternion rotation;
    std::map<std::string, float> morphWeights;
};

/*
 Drives a keyframe animation for a node.
 */
class VROKeyframeAnimation : public VROExecutableAnimation, public std::enable_shared_from_this<VROKeyframeAnimation> {
    
public:

    VROKeyframeAnimation(std::vector<std::unique_ptr<VROKeyframeAnimationFrame>> &frames,
                         float duration, bool hasTranslation, bool hasRotation, bool hasScale,
                         float hasMorphWeights) {
        _hasTranslation = hasTranslation;
        _hasRotation = hasRotation;
        _hasScale = hasScale;
        _frames = std::move(frames);
        _duration = duration;
        _hasMorphWeights = hasMorphWeights;
    }

    virtual ~VROKeyframeAnimation() { }
    
    void setName(std::string name) {
        _name = name;
    }
    std::string getName() const {
        return _name;
    }

    const std::vector<std::unique_ptr<VROKeyframeAnimationFrame>> &getFrames() const {
        return _frames;
    }

    void setDuration(float durationSeconds) {
        _duration = durationSeconds;
    }

    float getDuration() const {
        return _duration;
    }

    void setSpeed(float speed);

#pragma mark - Executable Animation API
    
    /*
     Produce a copy of this animation.
     */
    std::shared_ptr<VROExecutableAnimation> copy();
    
    /*
     Execute this animation. The onFinished() callback will be invoked when the
     animation is fully executed (if it has children, this is when the last child
     finishes executing).
     */
    void execute(std::shared_ptr<VRONode> node, std::function<void()> onFinished);
    void pause();
    void resume();
    void terminate(bool jumpToEnd);
    void preload() {}
    
    std::string toString() const;
private:
    
    /*
     The name of this animation.
     */
    std::string _name;
    
    /*
     The types of properties animated by this keyframe animation.
     */
    bool _hasTranslation, _hasRotation, _hasScale, _hasMorphWeights;

    /*
     The animation frames, in order of time.
     */
    std::vector<std::unique_ptr<VROKeyframeAnimationFrame>> _frames;
    
    /*
     The duration of this animation in seconds.
     */
    float _duration;
    
    /*
     If the animation is running, this is its associated transaction.
     */
    std::weak_ptr<VROTransaction> _transaction;
    
};

#endif /* VROKeyframeAnimation_h */
