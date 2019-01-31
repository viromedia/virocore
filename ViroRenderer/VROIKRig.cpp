//
//  VROIKRig.cpp
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROIKRig.h"
#include "VRONode.h"
#include "VROSkinner.h"
#include "VROBone.h"
#include "VROSkeleton.h"
static const float kFABRIKRetry = 50;
static const float kReachableEffectorThresholdMeters = 0.005;
static const bool kSyncNodePosition = false;
static const bool kLockIntermediaryBoneMode = true;

VROIKRig::VROIKRig(std::shared_ptr<VRONode> root,
                   std::map<std::string, std::shared_ptr<VRONode>> effectorNodeMap) {
    _rootJoint = std::make_shared<VROIKJoint>();
    _rootJoint->id = 1;
    _rootJoint->syncNode = root;
    _rootJoint->syncBone = -1;
    _allKnownIKJoints.push_back(_rootJoint);
    _skinner = nullptr;

    // Given the desired endEffectorNodeMap, create our map of end effector IKJoints
    for (auto effectorNode : effectorNodeMap) {
        std::shared_ptr<VROIKJoint> endJoint = nullptr;
        for (auto &joint : _allKnownIKJoints) {
            if (effectorNode.second == joint->syncNode) {
                endJoint = joint;
                break;
            }
        }

        if (endJoint == nullptr) {
            std::shared_ptr<VROIKJoint> effectorJoint = std::make_shared<VROIKJoint>();
            effectorJoint->id = _allKnownIKJoints.size() + 1;
            effectorJoint->position = effectorNode.second->getWorldPosition();
            effectorJoint->syncNode = effectorNode.second;
            effectorJoint->syncBone = -1;
            _allKnownIKJoints.push_back(effectorJoint);

            _keyToEffectorMap[effectorNode.first] = effectorJoint;
            _effectorTokeyMap[effectorJoint] = effectorNode.first;
            createSkeletalRigFromNodeTree(effectorNode.second);
        } else {
            _keyToEffectorMap[effectorNode.first] = endJoint;
            _effectorTokeyMap[endJoint] = effectorNode.first;
        }
     }

    // Lock all IKJoints that are in between effectors if needed.
    if (kLockIntermediaryBoneMode) {
        flagLockedJoints(_rootJoint, _rootJoint);
    }

    _initializeRig = true;
    _processedNewEffectorPositions = false;
}

VROIKRig::VROIKRig(std::shared_ptr<VROSkinner> skinner,
                   std::map<std::string, int> endEffectorBoneIndexMap) {
    _rootJoint = std::make_shared<VROIKJoint>();
    _rootJoint->id = 1;
    _rootJoint->syncNode = skinner->getSkinnerNode();
    _rootJoint->syncBone = 0;
    _allKnownIKJoints.push_back(_rootJoint);
    _skinner = skinner;

    // Given the desired endEffectorBoneIndexMap, create our map of end effector IKJoints
    for (auto endAffectorBoneId : endEffectorBoneIndexMap) {
        std::shared_ptr<VROIKJoint> endJoint = nullptr;
        for (auto &joint : _allKnownIKJoints) {
            if (endAffectorBoneId.second == joint->syncBone) {
                endJoint = joint;
                break;
            }
        }

        if (endJoint == nullptr) {
            VROMatrix4f boneTransform = skinner->getSkeleton()->getCurrentBoneWorldTransform(endAffectorBoneId.second);
            std::shared_ptr<VROIKJoint> effectorJoint = std::make_shared<VROIKJoint>();
            effectorJoint->id = _allKnownIKJoints.size() + 1;
            effectorJoint->position = boneTransform.extractTranslation();
            effectorJoint->syncNode = nullptr;
            effectorJoint->syncBone = endAffectorBoneId.second;
            _allKnownIKJoints.push_back(effectorJoint);

            _keyToEffectorMap[endAffectorBoneId.first] = effectorJoint;
            _effectorTokeyMap[effectorJoint] = endAffectorBoneId.first;
            createSkeletalRigFromSkinner(endAffectorBoneId.second);
        } else {
            _keyToEffectorMap[endAffectorBoneId.first] = endJoint;
            _effectorTokeyMap[endJoint] = endAffectorBoneId.first;
        }
    }

    // Lock all IKJoints that are in between effectors if needed.
    if (kLockIntermediaryBoneMode) {
        flagLockedJoints(_rootJoint, _rootJoint);
    }

    _initializeRig = true;
    _processedNewEffectorPositions = false;
}

VROIKRig::~VROIKRig() {
    // No-op
}

void VROIKRig::createSkeletalRigFromNodeTree(std::shared_ptr<VRONode> currentNode) {
    passert(currentNode != nullptr);

    // Determine if we have previously created an IKJoint for this node.
    std::shared_ptr<VROIKJoint> currentJoint = nullptr;
    for (auto &joint : _allKnownIKJoints) {
        if (joint->syncNode == currentNode) {
            currentJoint = joint;
            break;
        }
    }

    // Create an IKJoint from the given currentNode if we haven't yet done so.
    if (currentJoint == nullptr) {
        currentJoint = std::make_shared<VROIKJoint>();
        currentJoint->id = _allKnownIKJoints.size() + 1;
        currentJoint->position = currentNode->getWorldPosition();
        currentJoint->syncNode = currentNode;
        currentJoint->syncBone = -1;
        _allKnownIKJoints.push_back(currentJoint);
    }

    // Attach any known child joints by examining previously added IKJoints.
    std::vector<std::shared_ptr<VROIKJoint>> childJoints;
    for (auto &joint : _allKnownIKJoints) {
        for (auto &childNode : currentNode->getChildNodes()) {
            if (childNode == joint->syncNode) {
                childJoints.push_back(joint);
            }
        }
    }

    // Now hook up the connections for the current joint.
    currentJoint->children.clear();
    for (auto childJoint : childJoints) {
        childJoint->parent = currentJoint;
        currentJoint->children.push_back(childJoint);
    }

    // Return if we are at root, else continue recursing up the tree.
    if (currentJoint == _rootJoint) {
        return;
    } else {
        createSkeletalRigFromNodeTree(currentNode->getParentNode());
    }
}

void VROIKRig::flagLockedJoints(std::shared_ptr<VROIKJoint> referenceJoint,
                                std::shared_ptr<VROIKJoint> currentJoint) {
    if (currentJoint->children.size() == 0) {
        return;
    }

    bool isEffectorPoint = false;
    if (_effectorTokeyMap.find(currentJoint) != _effectorTokeyMap.end()) {
         isEffectorPoint = true;
    }

    // Recurse if we are at an effector point or junction point.
    if (isEffectorPoint || currentJoint == _rootJoint || currentJoint->children.size() > 1) {
        for (auto childJoint : currentJoint->children) {
            flagLockedJoints(currentJoint, childJoint);
        }
        return;
    }

    // Else, lock it and update child / parent references.
    detachIKJoint(currentJoint);
    if (referenceJoint->lockedJoints.size() > 0) {
        std::shared_ptr<VROIKJoint> parentLockedJoint = referenceJoint->lockedJoints.back();
        parentLockedJoint->children.clear();
        parentLockedJoint->children.push_back(currentJoint);
        currentJoint->parent = parentLockedJoint;
    }
    referenceJoint->lockedJoints.push_back(currentJoint);

    // Continue recursing down the rig
    for (auto &childJoint : currentJoint->children) {
        flagLockedJoints(referenceJoint, childJoint);
    }
}

void VROIKRig::detachIKJoint(std::shared_ptr<VROIKJoint> currentJoint) {
    // Update the parent joint references, if any.
    std::shared_ptr<VROIKJoint> parentJoint = currentJoint->parent;
    if (parentJoint != nullptr) {
        std::vector<std::shared_ptr<VROIKJoint>> &parentJointChildList = parentJoint->children;
        parentJointChildList.erase(std::remove_if(parentJointChildList.begin(), parentJointChildList.end(),
                       [currentJoint](const std::shared_ptr<VROIKJoint> & j) {
                           return j == currentJoint;
                       }), parentJointChildList.end());
    }

    // Update child joint references.
    for (auto &childJoint : currentJoint->children) {
        childJoint->parent = currentJoint->parent;
        currentJoint->parent->children.push_back(childJoint);
    }

    // Finally remove references in _allKnownIKJoints
    _allKnownIKJoints.erase(std::remove_if(_allKnownIKJoints.begin(), _allKnownIKJoints.end(),
                   [currentJoint](const std::shared_ptr<VROIKJoint> & j) {
                       return j == currentJoint;
                   }), _allKnownIKJoints.end());
}

void VROIKRig::createSkeletalRigFromSkinner(int boneId) {
    passert(boneId != -1);

    // Determine if we have previously created an IKJoint for this bone.
    std::shared_ptr<VROIKJoint> currentJoint = nullptr;
    for (auto &joint : _allKnownIKJoints) {
        if (joint->syncBone == boneId) {
            currentJoint = joint;
            break;
        }
    }

    // Create an IKJoint from the given current bone if we haven't yet done so.
    if (currentJoint == nullptr) {
        VROMatrix4f boneTransform = _skinner->getSkeleton()->getCurrentBoneWorldTransform(boneId);
        VROVector3f worldPos = boneTransform.extractTranslation();
        currentJoint = std::make_shared<VROIKJoint>();
        currentJoint->id = _allKnownIKJoints.size() + 1;
        currentJoint->position = worldPos;
        currentJoint->syncNode = nullptr;
        currentJoint->syncBone = boneId;
        _allKnownIKJoints.push_back(currentJoint);
    }

    // Attach any known child joints by examining previously added IKJoints.
    std::vector<std::shared_ptr<VROIKJoint>> childJoints;
    std::shared_ptr<VROSkeleton> skeleton = _skinner->getSkeleton();
    for (auto &joint : _allKnownIKJoints) {

        // Get all the child node for this current bone.
        std::vector<int> childBoneIds;
        for(int i = 0; i < skeleton->getNumBones(); i ++) {
            std::shared_ptr<VROBone> bone = skeleton->getBone(i);
            if (bone->getParentIndex() == boneId) {
                childBoneIds.push_back(i);
            }
        }

        // Now compare the child nodes to against joint.
        for (auto childId : childBoneIds) {
            if (childId == joint->syncBone) {
                childJoints.push_back(joint);
            }
        }
    }

    // Now hook up the connections for the current joint.
    currentJoint->children.clear();
    for (auto childJoint : childJoints) {
        if (childJoint == currentJoint) {
            continue;
        }
        childJoint->parent = currentJoint;
        currentJoint->children.push_back(childJoint);
    }

    // Return if we are at root, else continue recursing up the tree.
    if (currentJoint == _rootJoint) {
        return;
    } else {
        createSkeletalRigFromSkinner(skeleton->getBone(boneId)->getParentIndex());
    }
}

void VROIKRig::setPositionForEffector(std::string affectorId, VROVector3f pos) {
    if (_keyToEffectorMap.find(affectorId) == _keyToEffectorMap.end()) {
        pwarn("Attempted to set unknown affector id %s", affectorId.c_str());
        return;
    }
    _effectorDesiredPositionMap[affectorId] = pos;
    _processedNewEffectorPositions = false;
}

void VROIKRig::processRig() {
    if (_initializeRig) {
        initializeRig();
        return;
    }

    // Process root motion by examining the root joint's node or bone transform
    VROMatrix4f newRootPost = _rootJoint->syncNode->getWorldTransform();
    if (_skinner != nullptr) {
        newRootPost = newRootPost.multiply(_skinnerJointToRootBone);
    }
    _rootJoint->position = newRootPost.extractTranslation();

    // If we have already process the effector's newly set positions, return.
    if (_processedNewEffectorPositions) {
        return;
    }

    // Else, start our processing the IK calculations for this rig.
    processInverseKinematics();

    // Sync the results from the IK calculations.
    if (_skinner != nullptr) {
        syncResultSkinner(_rootJoint);
    } else {
        if (kSyncNodePosition) {
            syncResultPositionOnly(_rootJoint);
        } else {
            syncResultRotationOnly(_rootJoint);
        }
    }

    _processedNewEffectorPositions = true;
}

void VROIKRig::initializeRig() {
    if (_allKnownIKJoints.size() <=1) {
        pabort("Unable to initialize improperly configured rig.");
    }

    // First, refresh the rig with the latest joint positions. Assume that the node tree / bone world
    // transforms have been computed.
    for (auto &joint : _allKnownIKJoints) {
        if (_skinner) {
            VROMatrix4f mat = _skinner->getSkeleton()->getCurrentBoneWorldTransform(joint->syncBone);
            joint->position = mat.extractTranslation();
        } else {
            joint->position = joint->syncNode->getWorldPosition();
        }
    }

    // Also refresh our joint effector's desired positions.
    for (auto endAffectorJoint : _keyToEffectorMap) {
        std::string key = endAffectorJoint.first;
        if (_effectorDesiredPositionMap.find(key) != _effectorDesiredPositionMap.end()) {
            // only refresh rig pose for end effectors not in key.
            continue;
        }

        VROVector3f updatePos = _keyToEffectorMap[key]->position;
        _effectorDesiredPositionMap[key] = updatePos;
    }

    // Then start forming chains by combining into a vec of updated joints.
    _allKnownChains.clear();
    for (auto childJoint : _rootJoint->children) {
        std::shared_ptr<VROIKChain> chain = std::make_shared<VROIKChain>();
        chain->chainJoints.push_back(_rootJoint);
        chain->parentChain = nullptr;
        chain->totalLength = 0;
        _rootChains.push_back(chain);
        formChains(_rootJoint, childJoint, chain);
    }

    /*
     Now examine created chains to determine inter-chain dependencies.
     Do so by creating a tree graph that tracks which chains are dependent on each other.
     This will enable the case where we have multiple end affectors with
     connecting subnodes. Start with chains beginning from the root of the scene.
     */
    std::shared_ptr<VROIKChain> rootChain = nullptr;
    for (auto rootChain : _rootChains) {
        formChainDependencies(rootChain);
    }

    // Finally, examine all known chains, and link them to our effectors.
    for (auto effectorJoint : _keyToEffectorMap) {
        for (auto chain : _allKnownChains) {
            std::shared_ptr<VROIKJoint> currentJoint = chain->chainJoints.back();
            if ((_skinner != nullptr && currentJoint->syncBone == effectorJoint.second->syncBone) ||
                    (_skinner == nullptr && currentJoint->syncNode == effectorJoint.second->syncNode)) {
                _endEffectorIdToChains[currentJoint->id] = chain;
                break;
            }
        }

        // If the effector is not a leaf, it is an intermediate joint effector.
        std::shared_ptr<VROIKJoint> joint = effectorJoint.second;
        if (joint->children.size() > 0) {
            joint->isIntermediaryEffector = true;
        }
    }

    // If we have a root skinner bone, calculate any additional offsets the bone may have
    // from the skinner's node.
    if (_skinner != nullptr) {
        VROMatrix4f rootBoneTrans = _skinner->getSkeleton()->getCurrentBoneWorldTransform(0);
        VROMatrix4f skinnerTrans = _rootJoint->syncNode->getWorldTransform();

        _skinnerJointToRootBone.toIdentity();
        VROMatrix4f skinnerJointToRootBone = skinnerTrans.invert().multiply(rootBoneTrans);
        _skinnerJointToRootBone = skinnerJointToRootBone;
    }

    // Retain local joint references needed for rotation recalculations
    for (auto &joint : _allKnownIKJoints) {
        // Retain a reference to locked joint's local transforms, if any.
        for (auto &lockedJoint : joint->lockedJoints) {
            VROMatrix4f localTrans = getJointLocalTransform(lockedJoint);
            joint->lockedJointLocalTransforms.push_back(localTrans);
        }
        passert(joint->lockedJointLocalTransforms.size() == joint->lockedJoints.size());

        // Else always retain transforms for leaf joints.
        if (joint->children.size() == 0) {
            VROMatrix4f localTrans = getJointLocalTransform(joint);
            _endEffectorIdLocalRotation[joint->id] = localTrans.extractRotation(localTrans.extractScale()).getMatrix();
        }
    }

    _initializeRig = false;
}

VROMatrix4f VROIKRig::getJointLocalTransform(std::shared_ptr<VROIKJoint> referenceJoint) {
    VROMatrix4f jointTrans;
    VROMatrix4f parentTrans;
    if (referenceJoint->syncBone == -1) {
        jointTrans = referenceJoint->syncNode->getWorldTransform();

        if (referenceJoint->parent == nullptr) {
            return jointTrans;
        }

        parentTrans = referenceJoint->parent->syncNode->getWorldTransform();
    } else {
        int boneIndex = referenceJoint->syncBone;
        jointTrans = _skinner->getSkeleton()->getCurrentBoneWorldTransform(boneIndex);

        if (referenceJoint->parent == nullptr) {
            return jointTrans;
        }

        int parentIndex = referenceJoint->parent->syncBone;
        parentTrans = _skinner->getSkeleton()->getCurrentBoneWorldTransform(parentIndex);
    }

    return parentTrans.invert() * jointTrans;
}

void VROIKRig::formChains(std::shared_ptr<VROIKJoint> jointStart,
                          std::shared_ptr<VROIKJoint> currentJoint,
                          std::shared_ptr<VROIKChain> &currentChain) {
    // Update the currentChain
    currentChain->chainJoints.push_back(currentJoint);

    // Save distances between joints as bone length.
    VROVector3f currentPos = currentJoint->position;
    VROVector3f parentpos = currentJoint->parent->position;
    float d = currentPos.distanceAccurate(parentpos);
    currentChain->boneLengths.push_back(d);
    currentChain->totalLength += d;

    // Move on to child nodes, if any.
    int childCount = currentJoint->children.size();

    // If we have no child, we have encountered a leaf in the graph.
    // Thus, finalize and add the chain with the last known branchNodeStart point.
    if (childCount == 0) {
        _allKnownChains.push_back(currentChain);
        return;
    }

    // If we have only one child, continue retaining known branchNodeStart point.
    // and move on to the next node in the tree.
    if (childCount == 1) {
        std::shared_ptr<VROIKJoint> childJoint = currentJoint->children.front();
        formChains(jointStart, childJoint, currentChain);
        return;
    }

    // Each time we encounter a node that branches of in the graph, we always
    // solidify the previously known branch, before recursing down
    // the new subtree with a new known branchNodeStart.
    if (childCount > 1) {
        _allKnownChains.push_back(currentChain);
        for (auto childJoint : currentJoint->children) {
            std::shared_ptr<VROIKJoint> newBranchNodeStart = currentJoint;
            std::shared_ptr<VROIKChain> newChain = std::make_shared<VROIKChain>();
            newChain->chainJoints.push_back(newBranchNodeStart);
            newChain->parentChain = nullptr;
            newChain->totalLength = 0;
            formChains(newBranchNodeStart, childJoint, newChain);
        }
        return;
    }
}

void VROIKRig::formChainDependencies(std::shared_ptr<VROIKChain> &currentChain) {
    std::shared_ptr<VROIKJoint> currentEndNode = currentChain->chainJoints.back();

    // Identify all dependent chains beginning with a node that matches the
    // ending node of the currentChain.
    for(auto &possibleChildChain : _allKnownChains) {
        if (possibleChildChain == currentChain) {
            continue;
        }

        std::shared_ptr<VROIKJoint> chainBeginningNode = possibleChildChain->chainJoints.front();
        if (chainBeginningNode == currentEndNode) {
            currentChain->childChains.push_back(possibleChildChain);
            possibleChildChain->parentChain = currentChain;

            // Iterate down this child and link the rest up.
            formChainDependencies(possibleChildChain);
        }
    }

    if (currentChain->childChains.size() > 1) {
        currentChain->chainJoints.back()->isCentroidJoint = true;
    }
}

void VROIKRig::processInverseKinematics() {
    VROVector3f preservedRootPosition = _rootJoint->position;

    // Main FABRIK algorithm
    int retry = 0;
    while(retry < kFABRIKRetry) {
        // Step 0: Refresh fabric tree
        for (auto &chain : _allKnownChains) {
            chain->processed = false;
        }

        for (auto &joint : _allKnownIKJoints) {
            joint->centroidSubLocations.clear();
        }

        // Step 1: Re-Align the end effector point at the desired end locations
        for (auto affectorIdPos : _effectorDesiredPositionMap) {
            int jointId = _keyToEffectorMap[affectorIdPos.first]->id;

            // Only align those that are end effectors
            if (_keyToEffectorMap[affectorIdPos.first]->isIntermediaryEffector) {
                continue;
            }

            // Sanity check to ensure that all effectors are in this rig.
            if (_endEffectorIdToChains.find(jointId) == _endEffectorIdToChains.end()) {
                pabort("Attempted to process end affector that is not apart of this rig.");
            }

            std::shared_ptr<VROIKChain> chain = _endEffectorIdToChains[jointId];
            chain->chainJoints.back()->position = affectorIdPos.second;
        }

        // Step 2: Perform FABRIK Backwards to towards the root joint starting from
        // each end effector. Iterate through all effectors to ensure they are processed.
        for (auto endChain : _endEffectorIdToChains) {
            if (!endChain.second->processed){
                processChainTreeTowardsRoot(endChain.second);
            }
        }

        // Step 3: Now set the Root point back to the preserved position (it shouldn't move)
        _rootChains.front()->chainJoints.front()->position = preservedRootPosition;

        // Step 4: Perform FABRIK Forwards towards the end effectors.
        for (auto &chain : _rootChains) {
            processChainTreeTowardsEffectors(chain);
        }

        // Step 5: Now examine the end effectors and determine if they are close enough
        // to the desired target distance.
        if (hasEffectorsMetTarget()) {
           break;
        }

        // If not, repeat.
        retry ++;
    }
}

bool VROIKRig::hasEffectorsMetTarget() {
    for (auto desiredPos : _effectorDesiredPositionMap) {
        std::string key = desiredPos.first;
        std::shared_ptr<VROIKJoint> effectorJoint = _keyToEffectorMap[key];
        VROVector3f desiredPosition = desiredPos.second;
        VROVector3f currentPosition = effectorJoint->position;

        if (desiredPosition.distanceAccurate(currentPosition) > kReachableEffectorThresholdMeters) {
            return false;
        }
    }
    return true;
}

void VROIKRig::processChainTreeTowardsEffectors(std::shared_ptr<VROIKChain> &chain) {
    processFABRIKChainNode(chain, false);

    // Go through each chain's child and propagate the chain back.
    for (auto &childChain : chain->childChains) {
        processChainTreeTowardsEffectors(childChain);
    }
}

void VROIKRig::processChainTreeTowardsRoot(std::shared_ptr<VROIKChain> &chain) {
    if (chain->childChains.size() > 0) {
        // Identify which child has not yet been processed, and recurse on that node.
        for (auto childChain : chain->childChains) {
            if (!childChain->processed) {
                processChainTreeTowardsRoot(childChain);
            }
        }

        // Once all the child chains are processed, grab the joint at the beginning of the
        // child's chain, and then consolidate the results into a centroid point to be
        // added set on the last joint (the end) of the current chain.
        std::shared_ptr<VROIKJoint> compoundJoint = chain->chainJoints.back();
        VROVector3f centroidPosition = VROVector3f();
        for (VROVector3f pos : compoundJoint->centroidSubLocations) {
            centroidPosition = centroidPosition.add(pos);
        }

        centroidPosition = centroidPosition / chain->childChains.size();
        int end = chain->chainJoints.size() -1;
        chain->chainJoints[end]->position = centroidPosition;
    }

    processFABRIKChainNode(chain, true);
    chain->processed = true;

    // After processing this chain, move on to the parent chain node, return if at root.
    std::shared_ptr<VROIKChain> parentChain = chain->parentChain;
    if (parentChain != nullptr) {
        processChainTreeTowardsRoot(parentChain);
    }
}

void VROIKRig::processFABRIKChainNode(std::shared_ptr<VROIKChain> &chain,
                                      bool towardsRoot) {
    if (!towardsRoot) {
        for (int i = 0; i <= chain->chainJoints.size() - 2; i ++) {
            std::shared_ptr<VROIKJoint> &currentNode = chain->chainJoints[i];
            std::shared_ptr<VROIKJoint> &nextNode = chain->chainJoints[i + 1];

            VROVector3f nextNodePosition = nextNode->position;
            if (nextNode->isIntermediaryEffector) {
                std::string effectorKey = _effectorTokeyMap[nextNode];
                nextNodePosition = _effectorDesiredPositionMap[effectorKey];
            }

            VROVector3f dir = (nextNodePosition - currentNode->position).normalize();
            float distance = chain->boneLengths[i];
            VROVector3f newPos = currentNode->position.add(dir * distance);

            // Guard against weird conditions
            if (isnan(newPos.x) || isnan(newPos.y) || isnan(newPos.z)){
                continue;
            }
            nextNode->position = newPos;
        }
    } else {
        for (int i = chain->chainJoints.size() - 1; i >= 1; i --) {
            std::shared_ptr<VROIKJoint> &currentNode = chain->chainJoints[i];
            std::shared_ptr<VROIKJoint> &nextNode = chain->chainJoints[i - 1];

            VROVector3f nextNodePosition = nextNode->position;
            if (nextNode->isIntermediaryEffector) {
                std::string effectorKey = _effectorTokeyMap[nextNode];
                nextNodePosition = _effectorDesiredPositionMap[effectorKey];
            }

            VROVector3f dir = (nextNodePosition - currentNode->position).normalize();
            float distance = chain->boneLengths[i-1];
            VROVector3f newPos = currentNode->position.add(dir * distance);

            // Guard against weird conditions
            if (isnan(newPos.x) || isnan(newPos.y) || isnan(newPos.z)){
                newPos = nextNode->position;
            }

            if (nextNode->isCentroidJoint) {
                nextNode->centroidSubLocations.push_back(newPos);
            } else {
                nextNode->position = newPos;
            }
        }
    }
}

void VROIKRig::syncResultSkinner(std::shared_ptr<VROIKJoint> joint) {
    int currentBoneId = joint->syncBone;
    VROMatrix4f currentTransform = _skinner->getSkeleton()->getCurrentBoneWorldTransform(currentBoneId);
    VROVector3f currentScale = currentTransform.extractScale();
    VROQuaternion currentRot = currentTransform.extractRotation(currentScale);
    VROVector3f jointPos = joint->position;

    if (joint->children.size() == 0) {
        int parentJointBone = joint->parent->syncBone;
        VROMatrix4f parentTransform = _skinner->getSkeleton()->getCurrentBoneWorldTransform(parentJointBone);
        VROMatrix4f parentRot = parentTransform.extractRotation(parentTransform.extractScale()).getMatrix();

        // Create a matrix with the joint's positional transform
        VROMatrix4f mat;
        mat.toIdentity();
        mat.scale(currentScale.x, currentScale.y, currentScale.z);
        mat.rotate(parentRot);
        mat = _endEffectorIdLocalRotation[joint->id] * mat;
        mat.translate(jointPos);
        _skinner->getSkeleton()->setCurrentBoneWorldTransform(currentBoneId, mat, false);
        return;
    }

    if (joint->children.size() == 1) {
        VROVector3f jointChildPos = joint->children.front()->position;
        VROVector3f dir = (jointChildPos - jointPos).normalize();
        VROVector3f forwardWorld = {0, 1, 0};
        VROVector3f frm = currentRot * forwardWorld;
        VROQuaternion desiredRot =  VROQuaternion::rotationFromTo(frm, dir);

        // Create a matrix with the new position, and rotational joint transform
        VROMatrix4f mat;
        mat.toIdentity();
        mat.scale(currentScale.x, currentScale.y, currentScale.z);
        mat.rotate(currentRot);
        mat = desiredRot.getMatrix() * mat;
        mat.translate(jointPos);
        _skinner->getSkeleton()->setCurrentBoneWorldTransform(currentBoneId, mat, true);

        // Process intermediary locked joints, if any.
        syncLockedJoint(joint, mat);
    }

    if (joint->children.size() > 1) {
        // Create a matrix with the joint's positional transform
        VROMatrix4f mat;
        mat.toIdentity();
        mat.scale(currentScale.x, currentScale.y, currentScale.z);
        mat.rotate(currentRot);
        mat.translate(jointPos);
        _skinner->getSkeleton()->setCurrentBoneWorldTransform(currentBoneId, mat, true);

        // Process intermediary locked joints, if any.
        syncLockedJoint(joint, mat);
    }

    // Iterate down the IK tree
    for (auto &child : joint->children) {
        syncResultSkinner(child);
    }
}

void VROIKRig::syncResultRotationOnly(std::shared_ptr<VROIKJoint> joint) {
    if (joint->children.size() == 0) {
        VROVector3f jointPos = joint->position;
        VROQuaternion currentRot = joint->syncNode->getWorldRotation();
        joint->syncNode->setWorldTransform(jointPos, currentRot);
        return;
    }

    if (joint->children.size() == 1) {
        VROVector3f jointChildPos = joint->children.front()->position;
        VROVector3f jointPos = joint->position;
        VROVector3f dir = (jointChildPos - jointPos).normalize();

        VROMatrix4f currentWorldT = joint->syncNode->getWorldTransform();
        VROQuaternion currentRot = currentWorldT.extractRotation(currentWorldT.extractScale());
        VROVector3f currentScale = currentWorldT.extractScale();

        VROVector3f forwardWorld = {0, 1, 0};
        VROVector3f frm = currentRot * forwardWorld;
        VROQuaternion desiredRot =  VROQuaternion::rotationFromTo(frm, dir);

        // Create a matrix with the new position, and rotational joint transform
        VROMatrix4f mat;
        mat.toIdentity();
        mat.scale(currentScale.x, currentScale.y, currentScale.z);
        mat.rotate(currentRot);
        mat = desiredRot.getMatrix() * mat;
        mat.translate(jointPos);
        joint->syncNode->setWorldTransform(mat.extractTranslation(), mat.extractRotation(mat.extractScale()));

        // Process intermediary locked joints, if any.
        VROMatrix4f parentTrans = joint->syncNode->getWorldTransform();
        syncLockedJoint(joint, parentTrans);
    }

    if (joint->children.size() > 1) {
        VROVector3f jointPos = joint->position;
        VROQuaternion currentRot = joint->syncNode->getWorldRotation();
        joint->syncNode->setWorldTransform(jointPos, currentRot);

        // Process intermediary locked joints, if any.
        VROMatrix4f parentTrans = joint->syncNode->getWorldTransform();
        syncLockedJoint(joint, parentTrans);
    }

    for (auto &child : joint->children) {
        syncResultRotationOnly(child);
    }
}

void VROIKRig::syncResultPositionOnly(std::shared_ptr<VROIKJoint> joint) {
    VROVector3f pos = joint->position;
    VROQuaternion q = VROQuaternion();
    q.makeIdentity();
    joint->syncNode->setWorldTransform(pos, q);
    for (auto &child : joint->children) {
        syncResultPositionOnly(child);
    }
}

void VROIKRig::syncLockedJoint(std::shared_ptr<VROIKJoint> joint, VROMatrix4f parentTrans) {
    for (int i = 0; i < joint->lockedJoints.size(); i ++) {
        std::shared_ptr<VROIKJoint> lockedJoint = joint->lockedJoints[i];
        VROMatrix4f localTrans = joint->lockedJointLocalTransforms[i];
        VROMatrix4f finalTrans = parentTrans * localTrans;

        if (lockedJoint->syncBone == -1) {
            std::shared_ptr<VRONode> lockedNode = lockedJoint->syncNode;
            lockedNode->setWorldTransform(finalTrans.extractTranslation(),
                                          finalTrans.extractRotation(finalTrans.extractScale()));
            parentTrans = lockedNode->getWorldTransform();
        } else {
            int lockedBone = lockedJoint->syncBone;
            _skinner->getSkeleton()->setCurrentBoneWorldTransform(lockedBone, finalTrans, true);
            parentTrans = finalTrans;
        }
    }
}
