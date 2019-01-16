//
//  VROBodyTrackerController.cpp
//  ViroSample
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROBodyTrackerController.h"
#include "VROInputControllerAR.h"
#include "VROBodyTracker.h"
#include "VROMatrix4f.h"
#include "VROSkeleton.h"
#include "VROBone.h"
#include "VROBox.h"
#include "VROIKRig.h"
#include "VROProjector.h"
#if VRO_PLATFORM_IOS
#include "VROBodyTrackeriOS.h"
#include "VRODriverOpenGLiOS.h"
static std::string pointLabels[14] = {
    "top\t\t\t", //0
    "neck\t\t", //1
    "R shoulder\t", //2
    "R elbow\t\t", //3
    "R wrist\t\t", //4
    "L shoulder\t", //5
    "L elbow\t\t", //6
    "L wrist\t\t", //7
    "R hip\t\t", //8
    "R knee\t\t", //9
    "R ankle\t\t", //10
    "L hip\t\t", //11
    "L knee\t\t", //12
    "L ankle\t\t", //13
};

static UIColor *colors[14] = {
    [UIColor redColor],
    [UIColor greenColor],
    [UIColor blueColor],
    [UIColor cyanColor],
    [UIColor yellowColor],
    [UIColor magentaColor],
    [UIColor orangeColor],
    [UIColor purpleColor],
    [UIColor brownColor],
    [UIColor blackColor],
    [UIColor darkGrayColor],
    [UIColor lightGrayColor],
    [UIColor whiteColor],
    [UIColor grayColor]
};
#endif
static const float kHighConfidence = 0.45;
static const float kARHitTestWindowKernelPixel = 0.01;
static const float kVolatilityThresholdMeters = 0.15;
static const float kReachableBoneThresholdMeters = 0.2;
static const float kAutomaticSizingRatio = 0.85;
static const bool kModelDebugCubes = true;
static const bool kAutomaticResizing = true;
static const VROBodyJointType kBodyJointRoot = VROBodyJointType::Neck;

// Supported bone names within 3D models for ml joint tracking.
static const std::map<VROBodyJointType, std::string> kVROBodyBoneTags = {
  {VROBodyJointType::Top,             "Head"},
  {VROBodyJointType::Neck,            "Neck"},
  {VROBodyJointType::RightShoulder,   "RightShoulder"},
  {VROBodyJointType::RightElbow,      "RightElbow"},
  {VROBodyJointType::RightWrist,      "RightWrist"},
  {VROBodyJointType::RightHip,        "RightHip"},
  {VROBodyJointType::RightKnee,       "RightKnee"},
  {VROBodyJointType::RightAnkle,      "RightAnkle"},
  {VROBodyJointType::LeftShoulder,    "LeftShoulder"},
  {VROBodyJointType::LeftElbow,       "LeftElbow"},
  {VROBodyJointType::LeftWrist,       "LeftWrist"},
  {VROBodyJointType::LeftHip,         "LeftHip"},
  {VROBodyJointType::LeftKnee,        "LeftKnee"},
  {VROBodyJointType::LeftAnkle,       "LeftAnkle"},
};

VROBodyTrackerController::VROBodyTrackerController(std::shared_ptr<VRORenderer> renderer, std::shared_ptr<VRONode> sceneRoot) {
    _currentTrackedState = VROBodyTrackedState::NotAvailable;
    _rig = nullptr;
    _skinner = nullptr;
    _calibrating = false;
    _renderer = renderer;
    _mlRootToModelRoot = VROMatrix4f::identity();
    _bodyControllerRoot = std::make_shared<VRONode>();
    _calibrationEventDelegate = nullptr;

    sceneRoot->addChildNode(_bodyControllerRoot);
}

VROBodyTrackerController::~VROBodyTrackerController() {
}

bool VROBodyTrackerController::bindModel(std::shared_ptr<VRONode> modelRootNode) {
    _rig = nullptr;
    _keyToEffectorMap.clear();
    _skinner = nullptr;
    _modelRootNode = nullptr;
    _mlJointForBoneIndex.clear();
    _bodyControllerRoot->removeAllChildren();

    // Bind and initialize a model to this controller
    std::vector<std::shared_ptr<VROSkinner>> skinners;
    modelRootNode->getSkinner(skinners, true);
    if (skinners.size() == 0) {
        perror("VROBodyTrackerController: Attempted to bind to a model without a properly configured skinner.");
        return false;
    }

    /*
     Iterate through all known VROBodyJointType and examine the skinner's
     skeleton for bones required for body tracking. For each found bone,
     create an effector in our IKRig, and as well as joint data caches
     needed by this controller.
     */
    std::shared_ptr<VROSkeleton> skeleton = skinners[0]->getSkeleton();
    std::map<VROBodyJointType, std::string>::const_iterator bonePair;
    for (bonePair = kVROBodyBoneTags.begin(); bonePair != kVROBodyBoneTags.end(); bonePair++) {
        VROBodyJointType boneType = bonePair->first;
        std::string expectedBoneName = bonePair->second;

        // Determine if the model has a bone name matching the desired ML joint.
        std::shared_ptr<VROBone> bone = skeleton->getBone(expectedBoneName);
        if (bone == nullptr) {
            // Bone for ML joint does not exist in skeleton, continue.
            pwarn("Unable to find bone %s for VROBodyTrackerController!", expectedBoneName.c_str());
            continue;
        }

        int boneIndex = bone->getIndex();
        _mlJointForBoneIndex[boneType] = boneIndex;
        _keyToEffectorMap[expectedBoneName] = boneIndex;
    }

    // If we have not found required bones, fail the binding of the model.
    VROBodyJointType requiredBones[] = {kBodyJointRoot,
                                        VROBodyJointType::Neck,
                                        VROBodyJointType::LeftHip,
                                        VROBodyJointType::RightHip};
    for (auto requiredBone : requiredBones) {
        if (_mlJointForBoneIndex.find(requiredBone) == _mlJointForBoneIndex.end()) {
            perr("Attempted to bind 3D model with improperly configured bones to VROBodyTracker!");
            return false;
        }
    }

    // Else update our skinner references if we have the proper ML bones in our 3D model.
    _skinner = skinners[0];
    _modelRootNode = modelRootNode;
    _bodyControllerRoot->setScale(VROVector3f(1,1,1));

    // Set the model in it's original scale needed for determining ratios for automatic resizing.
    initializeModelUniformScale();

    // Create debug effector nodes UI
    _debugBoxEffectors.clear();
    for (auto bonePair : _mlJointForBoneIndex) {
        VROBodyJointType boneType = bonePair.first;
        std::string boneName = kVROBodyBoneTags.at(bonePair.first);
        VROVector3f pos = _skinner->getCurrentBoneWorldTransform(boneName).extractTranslation();
        std::shared_ptr<VRONode> block = createDebugBoxUI(true, boneName);
        _bodyControllerRoot->addChildNode(block);
        block->setWorldTransform(pos, VROMatrix4f::identity());
        _debugBoxEffectors[boneType] = block;
    }

    // Create a debug root node UI
    _debugBoxRoot = createDebugBoxUI(false, "Root");
    _bodyControllerRoot->addChildNode(_debugBoxRoot);

    // Bind calibration event delegates
    if (_calibrationEventDelegate == nullptr) {
        _calibrationEventDelegate = std::make_shared<VROBodyTrackerControllerEventDelegate>(shared_from_this());
        _calibrationEventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, false);
        _calibrationEventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnPinch, false);
        _modelRootNode->setEventDelegate(_calibrationEventDelegate);
    }

    // Set the timeout of joints in milliseconds.
    _mlJointTimeoutMap.clear();
    _mlJointTimeoutMap[VROBodyJointType::Top] =            500;
    _mlJointTimeoutMap[VROBodyJointType::Neck] =           800;
    _mlJointTimeoutMap[VROBodyJointType::LeftShoulder] =   500;
    _mlJointTimeoutMap[VROBodyJointType::LeftElbow] =      500;
    _mlJointTimeoutMap[VROBodyJointType::LeftWrist] =      500;
    _mlJointTimeoutMap[VROBodyJointType::RightShoulder] =  500;
    _mlJointTimeoutMap[VROBodyJointType::RightElbow] =     500;
    _mlJointTimeoutMap[VROBodyJointType::RightWrist] =     500;
    _mlJointTimeoutMap[VROBodyJointType::LeftHip] =        500;
    _mlJointTimeoutMap[VROBodyJointType::LeftKnee] =       500;
    _mlJointTimeoutMap[VROBodyJointType::LeftAnkle] =      500;
    _mlJointTimeoutMap[VROBodyJointType::RightHip] =       500;
    _mlJointTimeoutMap[VROBodyJointType::RightKnee] =      500;
    _mlJointTimeoutMap[VROBodyJointType::RightAnkle] =     500;
    return true;
}

void VROBodyTrackerController::initializeModelUniformScale() {
    // Set the model in it's original scale needed for determining ratios.
    VROVector3f currentScale = _modelRootNode->getScale();
    _modelRootNode->setScale(VROVector3f(1, 1, 1));
    std::shared_ptr<VRONode> parentNode = _modelRootNode->getParentNode();
    _modelRootNode->computeTransforms(parentNode->getWorldTransform(), parentNode->getWorldRotation());

    // Now calculate the ratios for automatic resizing.
    VROMatrix4f neckTrans =
            _skinner->getCurrentBoneWorldTransform(kVROBodyBoneTags.at(VROBodyJointType::Neck));
    VROMatrix4f leftHipTrans =
            _skinner->getCurrentBoneWorldTransform(kVROBodyBoneTags.at(VROBodyJointType::LeftHip));
    VROMatrix4f rightHipTrans =
            _skinner->getCurrentBoneWorldTransform(kVROBodyBoneTags.at(VROBodyJointType::RightHip));

    // Now get the middle of the hip.
    VROVector3f midVecFromLeft = (rightHipTrans.extractTranslation() - leftHipTrans.extractTranslation()).scale(0.5f);
    VROVector3f midHipLoc = leftHipTrans.extractTranslation().add(midVecFromLeft);
    VROVector3f neckLoc = neckTrans.extractTranslation();
    _originalNeckToHipDistance = midHipLoc.distanceAccurate(neckLoc);
}

void VROBodyTrackerController::startCalibration() {
    if (_skinner == nullptr) {
        pwarn("Unable to start calibration: Model has not yet been bounded to this controller!");
        return;
    }

    _calibrating = true;
    _calibrationEventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    _calibrationEventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnPinch, true);

    // reset the bones back to it's initial configuration
    std::shared_ptr<VROSkeleton> skeleton = _skinner->getSkeleton();
    for (int i = 0; i < skeleton->getNumBones(); i++) {
        std::shared_ptr<VROBone> bone = skeleton->getBone(i);
        bone->setTransform(VROMatrix4f::identity(), bone->getTransformType());
    }
}

void VROBodyTrackerController::finishCalibration() {
    if (_skinner == nullptr) {
        pwarn("Unable to finish calibration: Model has not yet been bounded to this controller!");
        return;
    }

    _rig = std::make_shared<VROIKRig>(_skinner, _keyToEffectorMap);
    _modelRootNode->setIKRig(_rig);

    // Start listening for new joint data.
    setBodyTrackedState(VROBodyTrackedState::NotAvailable);
    _calibrating = false;
    _calibrationEventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, false);
    _calibrationEventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnPinch, false);
}

void VROBodyTrackerController::onBodyJointsFound(const std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> &inferredJoints) {
    if (_modelRootNode == nullptr) {
        return;
    }
    
    // Convert to VROBodyJoint data structure, using only first joint of each type
    std::map<VROBodyJointType, VROBodyJoint> joints;
    for (auto &kv : inferredJoints) {
        VROInferredBodyJoint inferred = kv.second[0];
        
        VROBodyJoint joint = { inferred.getType(), inferred.getConfidence() };
        joint.setScreenCoords({ inferred.getBounds().getX(), inferred.getBounds().getY(), 0 });
        joint.setSpawnTimeMs(VROTimeCurrentMillis());
        
        joints[kv.first] = joint;
    }

    // Filter new joints found given by the VROBodyTracker and update _cachedTrackedJoints
    processJoints(joints);

#if VRO_PLATFORM_IOS
    updateDebugMLViewIOS(joints);
#endif

    if (_calibrating) {
        // Calibration requires a tracked ML Root Joint, return if we haven't yet found it.
        if (_currentTrackedState == NotAvailable) {
            return;
        }

        // Calculate a transform offset from a gLTF joint to the skinner's root node transform,
        // where the gLTF joint will represent the Body Tracked ML Root - kBodyJointRoot.
        // This transform offset will be needed for root motion re-alignment.
        int bodyJointRootAsgLTFJoint = _mlJointForBoneIndex[kBodyJointRoot];
        VROMatrix4f mlRootWorldTrans;
        mlRootWorldTrans.translate(_skinner->getCurrentBoneWorldTransform(bodyJointRootAsgLTFJoint).extractTranslation());
        VROMatrix4f modelRootWorldTrans;
        modelRootWorldTrans.translate(_modelRootNode->getWorldTransform().extractTranslation());
        _mlRootToModelRoot = mlRootWorldTrans.invert().multiply(modelRootWorldTrans);

        // Align the 3D model to the latest calibrated position
        alignModelRootToMLRoot();

        // Then scale the model to the right size.
        alignModelTorsoScale();
    } else {
        // If we are not calibrating, update tracked joints as usual
        updateModel();
    }

    // Render debug UI
    if (kModelDebugCubes) {
        _debugBoxRoot->setWorldTransform(_modelRootNode->getWorldPosition(), _modelRootNode->getWorldRotation());

        // Render debug joint cubes.
        VROMatrix4f identity = VROMatrix4f::identity();
        std::map<VROBodyJointType, VROBodyJoint>::const_iterator cachedJoint;
        for (cachedJoint = _cachedTrackedJoints.begin(); cachedJoint != _cachedTrackedJoints.end(); cachedJoint++) {
           VROBodyJoint joint = cachedJoint->second;
           VROVector3f pos = joint.getProjectedTransform().extractTranslation();
           VROBodyJointType boneMLJointType = cachedJoint->first;
           _debugBoxEffectors[boneMLJointType]->setWorldTransform(pos, identity);
        }
    }
}

void VROBodyTrackerController::processJoints(const std::map<VROBodyJointType, VROBodyJoint> &joints) {
    // Grab all the 2D joints of high confidence for the targets we want.
    std::map<VROBodyJointType, VROBodyJoint> latestJoints;
    for (auto &kv : joints) {
        if (kv.second.getConfidence() > kHighConfidence) {
            latestJoints[kv.first] = kv.second;
        }
    }

    // First, convert the joints into 3d space.
    projectJointsInto3DSpace(latestJoints);

    // Flush out old tracked joints that have expired from _cachedTrackedJoints;
    std::vector<VROBodyJoint> expiredJoints;
    double currentTime = VROTimeCurrentMillis();
    for (auto it = _cachedTrackedJoints.cbegin(), next_it = it; it != _cachedTrackedJoints.cend(); it = next_it) {
        ++next_it;

        double dataTimeStamp = it->second.getSpawnTimeMs();
        double staleTime = _mlJointTimeoutMap[it->first];
        if (currentTime > dataTimeStamp + staleTime) {
            _cachedTrackedJoints.erase(it);
            expiredJoints.push_back(it->second);
        }
    }

    // Then, perform filtering and update our known set of cached joints.
    updateCachedJoints(latestJoints);

    // With the new found joints, update the current tracking state
    if (_cachedTrackedJoints.find(VROBodyJointType::Neck) == _cachedTrackedJoints.end()) {
        setBodyTrackedState(VROBodyTrackedState::NotAvailable);
    } else if (_cachedTrackedJoints.size() == _mlJointForBoneIndex.size()) {
        setBodyTrackedState(VROBodyTrackedState::FullEffectors);
    } else if (_cachedTrackedJoints.size() >= 1) {
        setBodyTrackedState(VROBodyTrackedState::LimitedEffectors);
    }

    // Finally, restore joints if possible using last known data (even if they are old).
    restoreMissingJoints(expiredJoints);
}

void VROBodyTrackerController::projectJointsInto3DSpace(std::map<VROBodyJointType, VROBodyJoint> &latestJoints) {
    // If calibrating, we'll need to grab the Z depth at which to position our projected plane.
    if (_calibrating) {
        if (latestJoints.find(kBodyJointRoot) == latestJoints.end()) {
            return;
        }

        // Perform a window depth test around the body joint root to get an average Z depth.
        VROBodyJoint rootJoint = latestJoints[kBodyJointRoot];
        VROVector3f screenCoord = rootJoint.getScreenCoords();
        VROMatrix4f projectedTrans = VROMatrix4f::identity();
        performWindowDepthTest(screenCoord.x, screenCoord.y, projectedTrans);

        // Update our projection plane
        _projectedPlanePosition = projectedTrans.extractTranslation();
        VROVector3f camPos = _renderer->getCamera().getPosition();
        _projectedPlaneNormal = (camPos - _projectedPlanePosition).normalize();
    }

    // Project the 2D joints into 3D coordinates as usual.
    for (auto &joint : latestJoints) {
        VROMatrix4f hitTransform;
        float pointX = joint.second.getScreenCoords().x;
        float pointY = joint.second.getScreenCoords().y;
        bool success = performUnprojectionToPlane(pointX, pointY, hitTransform);
        if (!success) {
            joint.second.clearPojectedTransform();
        } else {
            joint.second.setProjectedTransform(hitTransform);
        }
    }

    // Remove points that have failed projections from the map of latestJoints
    for (auto it = latestJoints.cbegin(), next_it = it; it != latestJoints.cend(); it = next_it) {
        ++next_it;
        if (!it->second.hasValidProjectedTransform()) {
            latestJoints.erase(it);
        }
    }
}

void VROBodyTrackerController::updateCachedJoints(std::map<VROBodyJointType, VROBodyJoint> &latestJoints) {
    // Ignore filters if we are in the calibration phase
    if (_calibrating) {
        for (auto &latestjointPair : latestJoints) {
            _cachedTrackedJoints[latestjointPair.first] = latestjointPair.second;
        }
        return;
    }

    // Else, apply stricter filters during the tracking phase.
    for (auto &latestjointPair : latestJoints) {
        VROBodyJoint currentJoint = latestjointPair.second;
        VROBodyJointType currentType = latestjointPair.first;
        bool hasTrackedJoint = _cachedTrackedJoints.find(currentType) != _cachedTrackedJoints.end();
        bool shouldCacheNewJoint = false;

        // Apply stricter filters for updating existing joints. Else simply cache the new joints.
        if (hasTrackedJoint) {
            // First, if we have seen this joint before, ignore new positions of high
            // volatility; don't update if the delta of the updated transform exceeds a
            // certain threshold.
            VROVector3f currentPos = currentJoint.getProjectedTransform().extractTranslation();
            VROVector3f oldPos = _cachedTrackedJoints[currentType].getProjectedTransform().extractTranslation();
            if (oldPos.distanceAccurate(currentPos) < kVolatilityThresholdMeters) {
                shouldCacheNewJoint = true;
            }

            // Else perform a parent check of this BodyJoint against the last skinner configuration
            // with a slightly more relaxed threshold.
            if (isTargetReachableFromParentBone(currentJoint)) {
                shouldCacheNewJoint = true;
            }
        } else {
            shouldCacheNewJoint = true;
        }

        // Finally cache the joint
        if (shouldCacheNewJoint) {
            _cachedTrackedJoints[latestjointPair.first] = latestjointPair.second;
        }
    }
}

void VROBodyTrackerController::restoreMissingJoints(std::vector<VROBodyJoint> expiredJoints) {
    // Ignore if we are currently calibrating the rig.
    if (_calibrating) {
        return;
    }

    // Attempt to recover missing joints by using old cached joint transforms.
    for (auto &expiredJoint : expiredJoints) {
        if (_cachedTrackedJoints.find(expiredJoint.getType()) != _cachedTrackedJoints.end()){
            continue;
        }

        VROBodyJointType currentType = expiredJoint.getType();

        // Restore by repositioning this joint from when we last saw it relative to the root.
        VROMatrix4f cacheJointTransFromRoot = _cachedEffectorRootOffsets[currentType];
        VROMatrix4f rootTransJoint = _cachedTrackedJoints.find(kBodyJointRoot)->second.getProjectedTransform();
        VROMatrix4f jointTrans = rootTransJoint.multiply(cacheJointTransFromRoot);
        expiredJoint.setProjectedTransform(jointTrans);
        _cachedTrackedJoints[currentType] = expiredJoint;
    }

    // With the updated transforms, cache a known set of _cachedEffectorRootOffsets
    if (_currentTrackedState != VROBodyTrackedState::NotAvailable) {
        _cachedEffectorRootOffsets.clear();
        VROMatrix4f rootJointTrans = _cachedTrackedJoints[VROBodyJointType::Neck].getProjectedTransform();

        for (auto joint : _cachedTrackedJoints) {
            if (joint.first == VROBodyJointType::Neck) {
                continue;
            }
            VROMatrix4f jointTrans = joint.second.getProjectedTransform();
            VROMatrix4f rootToJointTrans = rootJointTrans.invert().multiply(jointTrans);
            _cachedEffectorRootOffsets[joint.first] = rootToJointTrans;
        }
    }
}

void VROBodyTrackerController::alignModelRootToMLRoot() {
    // Grab the world transform of what we consider to be the Body Joint's Root.
    VROMatrix4f bodyJointRootTransform = _cachedTrackedJoints[kBodyJointRoot].getProjectedTransform();

    // Note: we just want the translational part of the ML's root transform as scale
    // and rotation doesn't matter at this point - they will be taken into account in the IKRig.
    VROMatrix4f bodyJointRootTransformTranslation;
    bodyJointRootTransformTranslation.translate(bodyJointRootTransform.extractTranslation());

    // Calculate the model's desired root location by multiplying the precalculated
    // _mlRootToModelRoot given the current bodyJointRootTransform.
    VROMatrix4f modelRootTransform = bodyJointRootTransformTranslation.multiply(_mlRootToModelRoot);

    // Update the model's node.
    VROVector3f pos = modelRootTransform.extractTranslation();
    VROQuaternion rot = modelRootTransform.extractRotation(modelRootTransform.extractScale());
    _modelRootNode->setWorldTransform(pos, rot, false);
}

void VROBodyTrackerController::alignModelTorsoScale() {
    // We'll need the current dimensions of the model for resizing calculations.
    if (!kAutomaticResizing || _skinner == nullptr) {
        return;
    }

    // Grab the current Neck to Hip distances from the ML Joint.
    if (!_cachedTrackedJoints[VROBodyJointType::Neck].hasValidProjectedTransform() ||
        !_cachedTrackedJoints[VROBodyJointType::LeftHip].hasValidProjectedTransform() ||
        !_cachedTrackedJoints[VROBodyJointType::RightHip].hasValidProjectedTransform()) {
        return;
    }

    VROVector3f neckPos = _cachedTrackedJoints[VROBodyJointType::Neck].getProjectedTransform().extractTranslation();
    VROVector3f leftHipPos = _cachedTrackedJoints[VROBodyJointType::LeftHip].getProjectedTransform().extractTranslation();
    VROVector3f rightHipPos = _cachedTrackedJoints[VROBodyJointType::RightHip].getProjectedTransform().extractTranslation();

    // Now get the middle of the hip.
    VROVector3f midVecFromLeft = (rightHipPos - leftHipPos).scale(0.5f);
    VROVector3f midHipLoc = leftHipPos.add(midVecFromLeft);
    float mlNeckToHipDistnace = midHipLoc.distanceAccurate(neckPos);

    // Calculate the different distances, grab the ratio.
    float modelToMLRatio = mlNeckToHipDistnace / _originalNeckToHipDistance * kAutomaticSizingRatio;

    // Apply that ratio to the scale of the model.
    _modelRootNode->setScale(VROVector3f(modelToMLRatio, modelToMLRatio, modelToMLRatio));
}

void VROBodyTrackerController::updateModel() {
    if (_rig == nullptr || _currentTrackedState == VROBodyTrackedState::NotAvailable) {
        return;
    }

    // Update the root motion of the rig.
    alignModelRootToMLRoot();

    // Now update all known rig joints.
    VROMatrix4f identity = VROMatrix4f::identity();
    std::map<VROBodyJointType, VROBodyJoint>::const_iterator cachedJoint;
    for (cachedJoint = _cachedTrackedJoints.begin(); cachedJoint != _cachedTrackedJoints.end(); cachedJoint++) {
        VROBodyJoint joint = cachedJoint->second;
        VROVector3f pos = joint.getProjectedTransform().extractTranslation();

        VROBodyJointType boneMLJointType = cachedJoint->first;
        std::string boneName = kVROBodyBoneTags.at(boneMLJointType);
        _rig->setPositionForEffector(boneName, pos);
    }
}

bool VROBodyTrackerController::performWindowDepthTest(float x, float y, VROMatrix4f &matOut) {
    float d = kARHitTestWindowKernelPixel;
    std::vector<VROVector3f> trials;
    trials.push_back(VROVector3f(x,     y, 0));
    trials.push_back(VROVector3f(x + d, y - d, 0));
    trials.push_back(VROVector3f(x - d, y - d, 0));
    trials.push_back(VROVector3f(x + d, y + d, 0));
    trials.push_back(VROVector3f(x - d, y + d, 0));

    VROVector3f total;
    float count = 0;
    for (auto t : trials) {
        VROMatrix4f estimate;
        if (performDepthTest(t.x, t.y, estimate)) {
            count = count + 1;
            total = total.add(estimate.extractTranslation());
        }
    }

    if (count == 0) {
        return false; // Hit test has failed.
    }

    total = total / count;
    matOut.toIdentity();
    matOut.translate(total);
    return true;
}

bool VROBodyTrackerController::performDepthTest(float x, float y, VROMatrix4f &matOut) {
    // Else use the usual ARHitTest to determine depth.
    float _currentProjection = 0.0037;
    int viewportWidth =_renderer->getCamera().getViewport().getWidth();
    int viewportHeight = _renderer->getCamera().getViewport().getHeight();
    std::vector<std::shared_ptr<VROARHitTestResult>> results;
#if VRO_PLATFORM_IOS
    VROVector3f transformed = { x * viewportWidth, y * viewportHeight, _currentProjection };
    results = [_view performARHitTestWithPoint:transformed.x y:transformed.y];
#endif

    std::shared_ptr<VROARHitTestResult> finalResult = nullptr;
    for (auto result: results) {
        if (finalResult == nullptr) {
            finalResult = result;
            continue;
        }

        VROARHitTestResultType currentType = result->getType();
        VROARHitTestResultType savedType = finalResult->getType();

        // NOTE: Stack rank ARHittest quality by:
        // ExistingPlaneUsingExtent > ExistingPlane > FeaturePoint
        // Grab the result based on the ranked types. If the types are the same, get
        // the close-est one.
        if (savedType == VROARHitTestResultType::FeaturePoint
            && (currentType == VROARHitTestResultType::ExistingPlane
            || currentType != VROARHitTestResultType::ExistingPlaneUsingExtent)) {
            finalResult = result;
        } else if (savedType == VROARHitTestResultType::ExistingPlane
                   && currentType == VROARHitTestResultType::ExistingPlaneUsingExtent) {
            finalResult = result;
        } else if (savedType == currentType && result->getDistance() < finalResult->getDistance()) {
            finalResult = result;
        }
    }

    if (finalResult == nullptr) {
        return false;
    } else {
        VROVector3f pos = finalResult->getWorldTransform().extractTranslation();
        VROMatrix4f out = VROMatrix4f::identity();
        out.translate(pos);
        matOut = out;
        return true;
    }
}

bool VROBodyTrackerController::isTargetReachableFromParentBone(VROBodyJoint targetJoint) {
    int currentIndex = _mlJointForBoneIndex[targetJoint.getType()];
    int parentIndex = _skinner->getSkeleton()->getBone(currentIndex)->getParentIndex();
    VROMatrix4f childTransform = _skinner->getCurrentBoneWorldTransform(currentIndex);
    VROMatrix4f parentTransform = _skinner->getCurrentBoneWorldTransform(parentIndex);
    VROMatrix4f currentTransform = targetJoint.getProjectedTransform();

    float maxDistance = parentTransform.extractTranslation().distanceAccurate(childTransform.extractTranslation());
    float estDistance = parentTransform.extractTranslation().distanceAccurate(currentTransform.extractTranslation());
    if (estDistance < (maxDistance + kReachableBoneThresholdMeters)) {
        return true;
    }
    return false;
}

bool VROBodyTrackerController::performUnprojectionToPlane(float x, float y, VROMatrix4f &matOut) {
    const VROCamera &camera = _renderer->getCamera();
    int viewport[4] = {0, 0, camera.getViewport().getWidth(), camera.getViewport().getHeight()};
    VROMatrix4f mvp = camera.getProjection().multiply(camera.getLookAtMatrix());
    x = viewport[2] * x;
    y = viewport[3] * y;

    // Compute the camera ray by unprojecting the point at the near clipping plane
    // and the far clipping plane.
    VROVector3f ncpScreen(x, y, 0.0);
    VROVector3f ncpWorld;
    if (!VROProjector::unproject(ncpScreen, mvp.getArray(), viewport, &ncpWorld)) {
        return false;
    }

    VROVector3f fcpScreen(x, y, 1.0);
    VROVector3f fcpWorld;
    if (!VROProjector::unproject(fcpScreen, mvp.getArray(), viewport, &fcpWorld)) {
        return false;
    }
    VROVector3f ray = fcpWorld.subtract(ncpWorld).normalize();

    // Find the intersection between the plane and the controller forward
    VROVector3f intersectionPoint;
    bool success = ray.rayIntersectPlane(_projectedPlanePosition, _projectedPlaneNormal, ncpWorld, &intersectionPoint);
    matOut.toIdentity();
    matOut.translate(intersectionPoint);
    return success;
}

void VROBodyTrackerController::setBodyTrackedState(VROBodyTrackedState state) {
    if (_currentTrackedState == state) {
        return;
    }

    _currentTrackedState = state;
    std::shared_ptr<VROBodyTrackerControllerDelegate> delegate = _delegate.lock();
    if (delegate != nullptr) {
        delegate->onBodyTrackStateUpdate(_currentTrackedState);
    }
}

void VROBodyTrackerController::setDelegate(std::shared_ptr<VROBodyTrackerControllerDelegate> delegate) {
    _delegate = delegate;
}

std::shared_ptr<VRONode> VROBodyTrackerController::createDebugBoxUI(bool isAffector, std::string tag) {
    // Create our debug box node
    float dimen = 0.005;
    if (isAffector) {
        dimen =  0.005;
    }
    std::shared_ptr<VROBox> box = VROBox::createBox(dimen, dimen, dimen);
    std::shared_ptr<VROMaterial> mat = std::make_shared<VROMaterial>();
    mat->setCullMode(VROCullMode::None);
    mat->setReadsFromDepthBuffer(false);
    mat->setWritesToDepthBuffer(false);
    if (isAffector) {
        mat->getDiffuse().setColor(VROVector4f(0.0, 1.0, 0, 1.0));
    } else {
        mat->getDiffuse().setColor(VROVector4f(1.0, 0, 0, 1.0));
    }

    std::vector<std::shared_ptr<VROMaterial>> mats;
    mats.push_back(mat);
    box->setMaterials(mats);

    std::shared_ptr<VRONode> debugNode = std::make_shared<VRONode>();
    debugNode->setGeometry(box);
    debugNode->setRenderingOrder(10);
    debugNode->setTag(tag);
    debugNode->setIgnoreEventHandling(true);

    return debugNode;
}

#if VRO_PLATFORM_IOS
void VROBodyTrackerController::enableDebugMLViewIOS(std::shared_ptr<VRODriver> driver) {
    if (_view != NULL) {
        return;
    }

    _view = (VROViewAR *) std::dynamic_pointer_cast<VRODriverOpenGLiOS>(driver)->getView();
    int endJointCount = static_cast<int>(VROBodyJointType::LeftAnkle);
    for (int i = static_cast<int>(VROBodyJointType::Top); i <= endJointCount; i++) {
        _bodyViews[i] = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 4, 4)];
        _bodyViews[i].backgroundColor = colors[i];
        _bodyViews[i].clipsToBounds = NO;

        _labelViews[i] = [[UILabel alloc] initWithFrame:CGRectMake(7, -3, 300, 8)];
        _labelViews[i].text = [NSString stringWithUTF8String:pointLabels[i].c_str()];
        _labelViews[i].textColor = colors[i];
        _labelViews[i].font = [UIFont preferredFontForTextStyle:UIFontTextStyleCaption2];
        [_labelViews[i] setFont:[UIFont systemFontOfSize:9]];
        [_bodyViews[i] addSubview:_labelViews[i]];
        [_view addSubview:_bodyViews[i]];
    }
}

void VROBodyTrackerController::updateDebugMLViewIOS(const std::map<VROBodyJointType, VROBodyJoint> &joints) {
    float minAlpha = 0.4;
    float maxAlpha = 1.0;
    float maxConfidence = 0.6;
    float minConfidence = 0.1;
    int viewWidth  = _view.frame.size.width;
    int viewHight = _view.frame.size.height;

    int endJointCount = static_cast<int>(VROBodyJointType::LeftAnkle);
    for (int i = static_cast<int>(VROBodyJointType::Top); i <= endJointCount; i++) {
        std::string labelTag = pointLabels[i] + " [N/A]";
        _labelViews[i].text =  [NSString stringWithUTF8String:labelTag.c_str()];
    }

    for (auto &kv : joints) {
        VROVector3f point = kv.second.getScreenCoords();
        VROVector3f transformed = { point.x * viewWidth, point.y * viewHight, 0 };
        // Only update the text for points that match our level of confidence.
        // Note that low confidence points are still rendered to ensure validity.
        if (kv.second.getConfidence() > kHighConfidence) {
            std::string labelTag = pointLabels[(int)kv.first] + " -> " + kv.second.getProjectedTransform().extractTranslation().toString();
            _labelViews[(int)kv.first].text = [NSString stringWithUTF8String:labelTag.c_str()];
        }
        _bodyViews[(int) kv.first].center = CGPointMake(transformed.x, transformed.y);
        _bodyViews[(int) kv.first].alpha = VROMathInterpolate(kv.second.getConfidence(), minConfidence, maxConfidence, minAlpha, maxAlpha);
    }
}
#endif
