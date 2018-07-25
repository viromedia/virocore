//
//  VROSkeletalAnimation.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/16/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROSkeletalAnimation_h
#define VROSkeletalAnimation_h

#include <memory>
#include <vector>
#include "VROMatrix4f.h"
#include "VROExecutableAnimation.h"

class VROShaderModifier;
class VROSkeleton;

/*
 Single frame of a skeletal animation. Identifies the bones
 to be animated and the transform to apply to each.
 */
struct VROSkeletalAnimationFrame {
    
    /*
     Start time of this frame. Defined between [0, 1], where 0
     is the start of the animation and 1.0 is the end.
     */
    float time;
    
    /*
     The indices of the bones that are animated this frame,
     and the corresponding transformation matrix to apply to
     each.
     
     The indices must correspond to the skeleton's bones
     array.
     */
    std::vector<int> boneIndices;
    std::vector<VROMatrix4f> boneTransforms;
    
};

/*
 Drives the animation of a skeleton. Skeletal animations are 
 achieved by animating the transform matrices of VROBones in
 a VROSkeleton. The VROSkinners associated with the skeleton 
 propagate these bone animations to geometries.
 */
class VROSkeletalAnimation : public VROExecutableAnimation, public std::enable_shared_from_this<VROSkeletalAnimation> {
    
public:
        
    VROSkeletalAnimation(std::shared_ptr<VROSkeleton> skeleton,
                         std::vector<std::unique_ptr<VROSkeletalAnimationFrame>> &frames,
                         float duration) :
        _skeleton(skeleton),
        _frames(std::move(frames)),
        _duration(duration) {}
    virtual ~VROSkeletalAnimation() { }
    
    void setName(std::string name) {
        _name = name;
    }
    std::string getName() const {
        return _name;
    }
    
    const std::vector<std::unique_ptr<VROSkeletalAnimationFrame>> &getFrames() const {
        return _frames;
    }
    
#pragma mark - Executable Animation API
    
    /*
     Produce a copy of this animation.
     */
    std::shared_ptr<VROExecutableAnimation> copy();
    
    /*
     Execute this animation. The onFinished() callback will be invoked when the
     animation is fully executed (if it has children, this is when the last child
     finishes executing).
     
     For skeletal animations, the input node parameter is ignored. Skeletal 
     animations are associated with a specific skeleton, and will animate all nodes
     connected to that skeleton.
     */
    void execute(std::shared_ptr<VRONode> node, std::function<void()> onFinished);
    void pause();
    void resume();
    void terminate(bool jumpToEnd);

    /*
     Override the duration of this skeletal animation, in seconds.
     */
    void setDuration(float durationSeconds);
    float getDuration() const;
    
    std::string toString() const;
    
private:
    
    /*
     The name of this animation.
     */
    std::string _name;
    
    /*
     The skeleton we are animating.
     */
    std::shared_ptr<VROSkeleton> _skeleton;
    
    /*
     The animation frames, in order of time.
     */
    std::vector<std::unique_ptr<VROSkeletalAnimationFrame>> _frames;
    
    /*
     The duration of this animation in seconds.
     */
    float _duration;
    
    /*
     If the animation is running, this is its associated transaction.
     */
    std::weak_ptr<VROTransaction> _transaction;
    
};

#endif /* VROSkeletalAnimation_h */
