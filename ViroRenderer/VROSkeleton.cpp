//
//  VROSkeleton.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSkeleton.h"
#include "VROBone.h"
#include "VROStringUtil.h"
#include "VROLog.h"
#include "VRONode.h"
#include "VROBoneConstraint.h"

VROSkeleton::VROSkeleton(std::vector<std::shared_ptr<VROBone>> bones) {
    _bones = bones;

    for (auto &bone : bones) {
        std::string boneName = bone->getName();
        if (VROStringUtil::trim(boneName).size() == 0) {
            continue;
        }

        _nameToBonesMap[boneName] = bone;
    }

    /*
     Construct attachment nodes from the model's skeleton, if any.
     */
    for (auto &bone : _bones) {
        if (bone->getAttachmentTransforms().size() == 0) {
            continue;
        }

        int boneIndex = bone->getIndex();
        for (auto &attachmentTransformPair : bone->getAttachmentTransforms()) {
            std::string attachmentKey = attachmentTransformPair.first;
            std::shared_ptr<VRONode> attachmentNode = std::make_shared<VRONode>();
            _boneNodeAttachments[boneIndex][attachmentKey] = attachmentNode;
        }
    }
}

void VROSkeleton::setModelRootNode(std::shared_ptr<VRONode> modelRootNode) {
    _modelRootNode_w = modelRootNode;

    // Iterate through and create a VROBoneConstraint for each boneNodeAttachment
    for (auto &bonePair : _boneNodeAttachments) {
        int boneIndex = bonePair.first;
        std::map<std::string, std::shared_ptr<VRONode>> keyToNodeAttachments = bonePair.second;
        for (auto &keyNodePair : keyToNodeAttachments) {
            std::shared_ptr<VRONode> node = keyNodePair.second;
            std::string attachmentKey = keyNodePair.first;

            // Add our attachement nodes to the root node of the model.
            modelRootNode->addChildNode(node);

            // Now grab the offset transform in model space.
            std::map<std::string, VROMatrix4f> attachmentTransforms = _bones[boneIndex]->getAttachmentTransforms();
            VROMatrix4f offsetTransformModelSpace = attachmentTransforms[attachmentKey];

            // Apply VROBoneConstraint to constrain the position of the attachment node to the bone.
            std::shared_ptr<VROBoneConstraint> skeletalConstraint =
                    std::make_shared<VROBoneConstraint>(shared_from_this(), boneIndex, offsetTransformModelSpace);
            node->addConstraint(skeletalConstraint);
        }
    }
}

std::shared_ptr<VRONode> VROSkeleton::getModelRootNode() {
    return _modelRootNode_w.lock();
}

VROMatrix4f VROSkeleton::getCurrentBoneWorldTransform(std::string boneName) {
    std::shared_ptr<VROBone> bone = getBone(boneName);
    if (bone == nullptr) {
        pwarn("Unable to find bone of name %s", boneName.c_str());
        return VROMatrix4f::identity();
    }
    return getCurrentBoneWorldTransform(bone);
}

VROMatrix4f VROSkeleton::getCurrentBoneWorldTransform(int boneId) {
    if (boneId >= getNumBones()) {
        pwarn("Unable to find bone index %d", boneId);
        return VROMatrix4f::identity();
    }
    std::shared_ptr<VROBone> bone = getBone(boneId);
    return getCurrentBoneWorldTransform(bone);
}

VROMatrix4f VROSkeleton::getCurrentBoneWorldTransform(std::shared_ptr<VROBone> bone) {
    // Grab the model's root node transform
    std::shared_ptr<VRONode> modelRootNode = _modelRootNode_w.lock();
    VROMatrix4f modelRootWorldTrans = VROMatrix4f::identity();
    if (modelRootNode != nullptr) {
        modelRootWorldTrans = modelRootNode->getWorldTransform();
    }

    // Grab the bone's transform, convert it into geometry and then finally world space.
    // TODO VIRO-4758: Account for Local Bones when setting bone transforms
    VROMatrix4f transform = VROMatrix4f::identity();
    switch(bone->getTransformType()) {
        case VROBoneTransformType::Legacy:
            transform = bone->getBindTransform().invert().multiply(bone->getTransform());
            break;
        case VROBoneTransformType::Concatenated:
            transform = bone->getTransform();
            break;
        default:
            return VROMatrix4f::identity();
    }

    return modelRootWorldTrans * transform;
}

void VROSkeleton::setCurrentBoneWorldTransform(int boneId, VROMatrix4f targetWorldTrans, bool recurse) {
    if (boneId >= getNumBones()) {
        pwarn("Unable to find bone index %d", boneId);
        return;
    }
    std::shared_ptr<VROBone> bone = getBone(boneId);
    return setCurrentBoneWorldTransform(bone, targetWorldTrans, recurse);
}

void VROSkeleton::setCurrentBoneWorldTransform(std::string boneName, VROMatrix4f targetWorldTrans, bool recurse) {
    std::shared_ptr<VROBone> bone = getBone(boneName);
    if (bone == nullptr) {
        pwarn("Unable to find bone of name %s", boneName.c_str());
        return;
    }
    return setCurrentBoneWorldTransform(bone, targetWorldTrans, recurse);
}

void VROSkeleton::setCurrentBoneWorldTransform(std::shared_ptr<VROBone> bone,
                                              VROMatrix4f targetWorldTrans,
                                              bool recurse) {
    // TODO VIRO-4758: Account for Local Bones when setting bone transforms
    if (bone->getTransformType() == VROBoneTransformType::Local) {
        pwarn("Unable to set world transform of Local bone types");
        return;
    }

    // First grab the original bone transform in case we'll need to use it later
    VROMatrix4f originalTransform = getCurrentBoneWorldTransform(bone);

    // Grab the model's root node transform
    VROMatrix4f modelRootWorldTrans = VROMatrix4f::identity();
    std::shared_ptr<VRONode> modelRootNode = _modelRootNode_w.lock();
    if (modelRootNode != nullptr) {
        modelRootWorldTrans = modelRootNode->getWorldTransform();
    }

    // Convert the desired targetWorldTransform into model space
    VROMatrix4f targetModelTrans = modelRootWorldTrans.invert().multiply(targetWorldTrans);
    VROMatrix4f boneTarget;
    switch(bone->getTransformType()) {
        case VROBoneTransformType::Legacy:
            boneTarget = bone->getBindTransform().multiply(targetModelTrans);
            break;
        case VROBoneTransformType::Concatenated:
            boneTarget = targetModelTrans;
            break;
        default:
            return;
    }
    bone->setTransform(boneTarget, bone->getTransformType());

    // If we are not recursing this calculation down to child bones, return.
    if (!recurse) {
        return;
    }

    // Grab all the child bones from the current boneId.
    int boneId = bone->getIndex();
    std::vector<int> childBoneIndexes;
    for (int i = 0; i < getNumBones(); i ++) {
        if (getBone(i)->getParentIndex() == boneId && i != boneId) {
            childBoneIndexes.push_back(i);
        }
    }

    if (childBoneIndexes.size() <=0) {
        return;
    }

    // Now calculate the child's transform in reference to the parent.
    for (auto childBoneIndex : childBoneIndexes) {
        VROMatrix4f childWorldTrans = getCurrentBoneWorldTransform(childBoneIndex);
        VROMatrix4f transformToChild = originalTransform.invert().multiply(childWorldTrans);
        VROMatrix4f newChildTransform = targetWorldTrans.multiply(transformToChild);
        setCurrentBoneWorldTransform(childBoneIndex, newChildTransform, recurse);
    }
}
