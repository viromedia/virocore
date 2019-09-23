//
//  VROSkeleton.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/10/17.
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

#ifndef VROSkeleton_h
#define VROSkeleton_h

class VROBone;
class VRONode;

#include <memory>
#include <vector>
#include <map>
#include <string>
#include "VROMatrix4f.h"
#include "VROVector3f.h"

/*
 The VROSkeleton is the bone-structure that defines the control points
 for a skeletal animation. These control points (the bones) are the animatable
 objects that drive skeletal animations.
 
 To associate a skeleton with a geometry, use a VROSkinner.
 */
class VROSkeleton : public std::enable_shared_from_this<VROSkeleton>{
    
public:
    
    VROSkeleton(std::vector<std::shared_ptr<VROBone>> bones);
    virtual ~VROSkeleton() {}
    
    int getNumBones() const {
        return (int)_bones.size();
    }

    const std::shared_ptr<VROBone> getBone(int index) const {
        return _bones[index];
    }

    const std::shared_ptr<VROBone> getBone(std::string name) const {
        auto bone = _nameToBonesMap.find(name);
        if (bone != _nameToBonesMap.end()) {
            return bone->second;
        }
        return nullptr;
    }

    /*
     Returns a map of bone attachment nodes associated with this skeleton.
     */
    const std::map<int, std::map<std::string, std::shared_ptr<VRONode>>> getBoneAttachments() const {
        return _boneNodeAttachments;
    }

    /*
     Returns the world transform of the given bone.
     TODO VIRO-4901: Implement propert bone binding transforms for gLTF models as well
     */
    VROMatrix4f getCurrentBoneWorldTransform(int boneId);
    VROMatrix4f getCurrentBoneWorldTransform(std::string boneName);
    VROMatrix4f getCurrentBoneWorldTransform(std::shared_ptr<VROBone> bone);

    /*
     Sets the world transform of the given bone.
     TODO VIRO-4901: Implement propert bone binding transforms for gLTF models as well
     */
    void setCurrentBoneWorldTransform(int boneId, VROMatrix4f transform, bool recurse);
    void setCurrentBoneWorldTransform(std::string boneName, VROMatrix4f transform, bool recurse);
    void setCurrentBoneWorldTransform(std::shared_ptr<VROBone> bone, VROMatrix4f transform, bool recurse);

    /*
     Sets a weak reference to the skinner's root for boneToWorld calculations.
     */
    void setSkinnerRootNode(std::shared_ptr<VRONode> modelRootNode);

    /*
     Returns the model's root node referenced by this VROSkeleton.
     */
    std::shared_ptr<VRONode> getSkinnerRootNode();

    /*
     Positions and scales all bones between and including the given parent and child bone
     with the given scaleFactor and in the given scaleDirection.
     */
    void scaleBoneTransforms(int startParentBone, int endChildBone,
                             float scaleFactor, VROVector3f scaleDirection);
private:
    /*
     The bones representing the skeleton. Each represents a control point of
     the animation. Moving a bone deforms the surface of this skinner's
     geometry.
     */
    std::vector<std::shared_ptr<VROBone>> _bones;

    /*
     A map of all known bone names to bones
     */
    std::map<std::string, std::shared_ptr<VROBone>> _nameToBonesMap;

    /*
     Model root reference for boneToWorld calculations.
     */
    std::weak_ptr<VRONode> _modelRootNode_w;

    /*
     A map of all bone attachment nodes in this skeleton in the form:
     <boneIndex, <attachmentTransformKey, attachmentNode>>
     */
    std::map<int, std::map<std::string, std::shared_ptr<VRONode>>> _boneNodeAttachments;

    /*
     Positions and scales the currentBone index with the given scaleFactor in the given
     scaleDirection. This is done recursively starting from the parent node moving outwards
     towards the child nodes.
     */
    void scaleBoneTransform(int currentBoneIndex, std::vector<int> &bonesChildFirst,
                            VROMatrix4f parentTransform, float scaleFactor,
                            VROVector3f scaleDirection);
};

#endif /* VROSkeleton_h */
