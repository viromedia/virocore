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
static const int kTotalEffectors = 6;
static const float kHighConfidence = 0.45;
static const float kARHitTestWindowKernelPixel = 0.005;
static const float kVolatilityThresholdMeters = 0.025;
static const float kReachableBoneThresholdMeters = 0.05;
static const VROBodyJointType kBodyJointRoot = VROBodyJointType::Neck;

void VROBodyTrackerController::bindModel(std::shared_ptr<VRONode> gltfRootNode) {
    _rig = nullptr;

    // if unbinding, reset all our model references.
    if (gltfRootNode == nullptr) {
        _skinner = nullptr;
        _modelRootNode = nullptr;
        _mlJointToModelJointMap.clear();
        _bodyControllerRoot->removeAllChildren();
        return;
    }

    // Else, bind and initialize a model to this controller
    std::vector<std::shared_ptr<VROSkinner>> skinners;
    gltfRootNode->getSkinner(skinners, true);
    if (skinners.size() != 1) {
        perror("VROBodyTrackerController: Attempted to bind to a model without a properly configured skinner.");
        return;
    }
    _skinner = skinners[0];
    _modelRootNode = gltfRootNode;
    _bodyControllerRoot->setScale(VROVector3f(1,1,1));
    _bodyControllerRoot->removeAllChildren();

    // Map the MLJoint to this model's bones.
    // TODO VIRO-4674: Hook up the effectors and joints automatically
    _mlJointToModelJointMap[VROBodyJointType::Top] =            4;
    _mlJointToModelJointMap[VROBodyJointType::Neck] =           2;
    _mlJointToModelJointMap[VROBodyJointType::LeftShoulder] =   5;
    _mlJointToModelJointMap[VROBodyJointType::LeftElbow] =      7;
    _mlJointToModelJointMap[VROBodyJointType::LeftWrist] =      9;
    _mlJointToModelJointMap[VROBodyJointType::RightShoulder] =  6;
    _mlJointToModelJointMap[VROBodyJointType::RightElbow] =     8;
    _mlJointToModelJointMap[VROBodyJointType::RightWrist] =     10;
    _mlJointToModelJointMap[VROBodyJointType::LeftHip] =        12;
    _mlJointToModelJointMap[VROBodyJointType::LeftKnee] =       14;
    _mlJointToModelJointMap[VROBodyJointType::LeftAnkle] =      16;
    _mlJointToModelJointMap[VROBodyJointType::RightHip] =       11;
    _mlJointToModelJointMap[VROBodyJointType::RightKnee] =      13;
    _mlJointToModelJointMap[VROBodyJointType::RightAnkle] =     15;

    // Map our bones on this model to the right effector
    // TODO VIRO-4674: Hook up the effectors and joints automatically
    _keyToEffectorMap.clear();
    _keyToEffectorMap["leftHand"] = 9;
    _keyToEffectorMap["rightHand"] = 10;
    _keyToEffectorMap["leftFeet"] = 17;
    _keyToEffectorMap["rightFeet"] = 18;
    _keyToEffectorMap["head"] = 4;

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

    // Create debug effector nodes UI
    _debugBoxEffectors.clear();
    for (auto ef : _keyToEffectorMap) {
        VROVector3f pos = _skinner->getCurrentBoneWorldTransform(ef.second).extractTranslation();
        std::shared_ptr<VRONode> block = createDebugBoxUI(true, ef.first);
        _bodyControllerRoot->addChildNode(block);
        block->setWorldTransform(pos, VROMatrix4f::identity());
        _debugBoxEffectors[ef.first] = block;
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
}

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

void VROBodyTrackerController::onBodyJointsFound(const std::map<VROBodyJointType, VROBodyJoint> &joints) {
    if (_modelRootNode == nullptr) {
        return;
    }

    // Filter new joints found given by the VROBodyTracker and update _cachedTrackedJoints
    updateTrackingJoints(joints);

#if VRO_PLATFORM_IOS
    updateDebugMLViewIOS();
#endif

    if (_calibrating) {
        // Calibration requires a tracked ML Root Joint, return if we haven't yet found it.
        if (_currentTrackedState == NotAvailable) {
            return;
        }

        // Calculate a transform offset from a gLTF joint to the skinner's root node transform,
        // where the gLTF joint will represent the Body Tracked ML Root - kBodyJointRoot.
        // This transform offset will be needed for root motion re-alignment.
        int bodyJointRootAsgLTFJoint = _mlJointToModelJointMap[kBodyJointRoot];
        VROMatrix4f mlRootWorldTrans;
        mlRootWorldTrans.translate(_skinner->getCurrentBoneWorldTransform(bodyJointRootAsgLTFJoint).extractTranslation());
        VROMatrix4f modelRootWorldTrans;
        modelRootWorldTrans.translate(_modelRootNode->getWorldTransform().extractTranslation());
        _mlRootToModelRoot = mlRootWorldTrans.invert().multiply(modelRootWorldTrans);

        // Align the 3D model to the latest calibrated position
        alignModelRootToMLRoot();
    } else {
        // If we are not calibrating, update tracked joints as usual
        updateModel();
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

void VROBodyTrackerController::startCalibration() {
    _calibrating = true;
    _calibrationEventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    _calibrationEventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnPinch, true);
    if (_skinner == nullptr) {
        return;
    }

    // reset the bones back to it's initial configuration
    std::shared_ptr<VROSkeleton> skeleton = _skinner->getSkeleton();
    for (int i = 0; i < skeleton->getNumBones(); i++) {
        skeleton->getBone(i)->setTransform(VROMatrix4f::identity(), VROBoneTransformType::Legacy);
    }
}

void VROBodyTrackerController::finishCalibration() {
    _rig = std::make_shared<VROIKRig>(_skinner, _keyToEffectorMap);
    _modelRootNode->setIKRig(_rig);

    // Start listening for new joint data.
    setBodyTrackedState(VROBodyTrackedState::NotAvailable);
    _calibrating = false;
    _calibrationEventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, false);
    _calibrationEventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnPinch, false);
}

void VROBodyTrackerController::setDelegate(std::shared_ptr<VROBodyTrackerControllerDelegate> delegate) {
    _delegate = delegate;
}

void VROBodyTrackerController::updateTrackingJoints(const std::map<VROBodyJointType, VROBodyJoint> &joints) {
    // Grab all the 2D joints of high confidence for the targets we want.
    std::map<VROBodyJointType, VROBodyJoint> latestJoints;
    for (auto &kv : joints) {
        if (kv.first != VROBodyJointType::Neck &&
            kv.first != VROBodyJointType::Top &&
            kv.first != VROBodyJointType::RightAnkle &&
            kv.first != VROBodyJointType::LeftAnkle &&
            kv.first != VROBodyJointType::RightWrist &&
            kv.first != VROBodyJointType::LeftWrist) {
            continue;
        }

        if (kv.second.getConfidence() > kHighConfidence) {
            latestJoints[kv.first] = kv.second;
        }
    }

    // Project the 2D joints into 3D coordinates via ARHitTests
    for (auto &joint : latestJoints) {
        VROMatrix4f hitTransform;
        float pointX = joint.second.getScreenCoords().x;
        float pointY = joint.second.getScreenCoords().y;
        bool success = performARHitTest(pointX, pointY, hitTransform);

        // If the ARHit test has failed for this joint, attempt to recover with
        // a more aggressive ARHitTest.
        VROMatrix4f updatedTransform = hitTransform;
        if (!success && !performARWindowHitTest(pointX,
                                                pointY,
                                                updatedTransform)) {
            joint.second.clearPojectedTransform();
        }

        joint.second.setProjectedTransform(updatedTransform);
    }

    // Remove points that have failed ar hit test from the map of latestJoints
    // TODO: Attempt to reconstruct/recover low confidence joints from adjacent high confidence ones.
    for (auto it = latestJoints.cbegin(), next_it = it; it != latestJoints.cend(); it = next_it) {
        ++next_it;
        if (!it->second.hasValidProjectedTransform()) {
            latestJoints.erase(it);
        }
    }

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

    // With the new found joints, update the current tracking state
    if (_cachedTrackedJoints.find(VROBodyJointType::Neck) == _cachedTrackedJoints.end()) {
        setBodyTrackedState(VROBodyTrackedState::NotAvailable);
    } else if (_cachedTrackedJoints.size() == kTotalEffectors) {
        setBodyTrackedState(VROBodyTrackedState::FullEffectors);
    } else if (_cachedTrackedJoints.size() >= 1) {
        setBodyTrackedState(VROBodyTrackedState::LimitedEffectors);
    }

    // Attempt to recover missing joints by using old cached joint transforms.
    for (auto &expiredJoint : expiredJoints) {
        VROBodyJointType currentType = expiredJoint.getType();
        bool hasIncomingData = latestJoints.find(currentType) != latestJoints.end();
        if (hasIncomingData) {
            // Ignore if the incoming latestJoints already has this information
            continue;
        }

        // Restore by repositioning this joint from when we last saw it relative to the root.
        VROMatrix4f cacheJointTransFromRoot = _cachedEffectorRootOffsets[currentType];
        VROMatrix4f rootTransJoint = _cachedTrackedJoints.find(
                VROBodyJointType::Neck)->second.getProjectedTransform();
        VROMatrix4f jointTrans = rootTransJoint.multiply(cacheJointTransFromRoot);
        expiredJoint.setProjectedTransform(jointTrans);
        _cachedTrackedJoints[currentType] = expiredJoint;
    }

    // At this stage, verify the validity of the new latestJoint transforms.
    for (auto &jointPair : latestJoints) {
        VROBodyJoint currentJoint = jointPair.second;
        VROBodyJointType currentType = jointPair.first;

        // First check and remove new joints of high volatility, mainly used for joints that we
        // have seen before. Don't update if the delta of the updated transform exceeds a
        // certain threshold.
        VROMatrix4f updatedTransform = currentJoint.getProjectedTransform();
        if (_cachedTrackedJoints.find(currentType) != _cachedTrackedJoints.end()) {
            VROVector3f oldPos = _cachedTrackedJoints[currentType].getProjectedTransform().extractTranslation();
            VROVector3f currentPos = updatedTransform.extractTranslation();
            if (oldPos.distanceAccurate(currentPos) > kVolatilityThresholdMeters) {
                continue;
            }
        }

        // If we have not seen this joint before, determine if it is possible to reach the
        // new location, consiering the joint's last position and it's bone length.
        if (!_calibrating && !isTargetReachableFromParentBone(jointPair.second, updatedTransform)) {
           continue;
        }

        // Finally, update _cachedTrackedJoints
        VROBodyJoint joint = jointPair.second;
        joint.setProjectedTransform(updatedTransform);
        _cachedTrackedJoints[jointPair.first] = joint;
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

bool VROBodyTrackerController::performARWindowHitTest(float x, float y, VROMatrix4f &matOut) {
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
        if (performARHitTest(t.x, t.y, estimate)) {
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

bool VROBodyTrackerController::performARHitTest(float x, float y, VROMatrix4f &matOut) {
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
        pwarn("Warning, no Results found for body tracking ARHittests.");
        return false;
    } else {
        VROVector3f pos = finalResult->getWorldTransform().extractTranslation();
        VROMatrix4f out = VROMatrix4f::identity();
        out.translate(pos);
        matOut = out;
        return true;
    }
}

bool VROBodyTrackerController::isTargetReachableFromParentBone(VROBodyJoint mlJoint, VROMatrix4f targetTransform) {
    int boneID = _mlJointToModelJointMap[mlJoint.getType()];
    int parentIndex = _skinner->getSkeleton()->getBone(boneID)->getParentIndex();
    VROMatrix4f childTransform = _skinner->getCurrentBoneWorldTransform(boneID);
    VROMatrix4f parentTransform = _skinner->getCurrentBoneWorldTransform(parentIndex);

    float maxDistance = parentTransform.extractTranslation().distanceAccurate(childTransform.extractTranslation());
    float estDistance = parentTransform.extractTranslation().distanceAccurate(targetTransform.extractTranslation());
    if (estDistance < (maxDistance + kReachableBoneThresholdMeters)) {
        return true;
    }
    return false;
}

void VROBodyTrackerController::updateModel() {
    if (_rig == nullptr || _currentTrackedState == VROBodyTrackedState::NotAvailable) {
        return;
    }

    // Update all joints
    alignModelRootToMLRoot();
    _debugBoxRoot->setWorldTransform(_modelRootNode->getWorldPosition(), _modelRootNode->getWorldRotation());

    VROMatrix4f i = VROMatrix4f::identity();
    if (_cachedTrackedJoints.find(VROBodyJointType::LeftWrist) != _cachedTrackedJoints.end()) {
        VROVector3f pos = _cachedTrackedJoints[VROBodyJointType::LeftWrist].getProjectedTransform().extractTranslation();
        _debugBoxEffectors["leftHand"]->setWorldTransform(pos, i);
        _rig->setPositionForEffector("leftHand", pos);
    }

    if (_cachedTrackedJoints.find(VROBodyJointType::RightWrist) != _cachedTrackedJoints.end()) {
        VROVector3f pos = _cachedTrackedJoints[VROBodyJointType::RightWrist].getProjectedTransform().extractTranslation();
        _debugBoxEffectors["rightHand"]->setWorldTransform(pos, i);
        _rig->setPositionForEffector("rightHand", pos);
    }

    if (_cachedTrackedJoints.find(VROBodyJointType::LeftAnkle) != _cachedTrackedJoints.end()) {
        VROVector3f pos = _cachedTrackedJoints[VROBodyJointType::LeftAnkle].getProjectedTransform().extractTranslation();
        _debugBoxEffectors["leftFeet"]->setWorldTransform(pos, i);
        _rig->setPositionForEffector("leftFeet", pos);
    }

    if (_cachedTrackedJoints.find(VROBodyJointType::RightAnkle) != _cachedTrackedJoints.end()) {
        VROVector3f pos = _cachedTrackedJoints[VROBodyJointType::RightAnkle].getProjectedTransform().extractTranslation();
        _debugBoxEffectors["rightFeet"]->setWorldTransform(pos, i);
        _rig->setPositionForEffector("rightFeet", pos);
    }

    if (_cachedTrackedJoints.find(VROBodyJointType::Top) != _cachedTrackedJoints.end()) {
        VROVector3f pos = _cachedTrackedJoints[VROBodyJointType::Top].getProjectedTransform().extractTranslation();
        _debugBoxEffectors["head"]->setWorldTransform(pos, i);
        _rig->setPositionForEffector("head", pos);
    }
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

void VROBodyTrackerController::updateDebugMLViewIOS() {
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

    for (auto &kv : _cachedTrackedJoints) {
        VROVector3f point = kv.second.getScreenCoords();
        VROVector3f transformed = { point.x * viewWidth, point.y * viewHight, 0 };
        std::string labelTag = pointLabels[(int)kv.first] + " -> " + kv.second.getProjectedTransform().extractTranslation().toString();
        _labelViews[(int)kv.first].text = [NSString stringWithUTF8String:labelTag.c_str()];
        _bodyViews[(int) kv.first].center = CGPointMake(transformed.x, transformed.y);
        _bodyViews[(int) kv.first].alpha = VROMathInterpolate(kv.second.getConfidence(), minConfidence, maxConfidence, minAlpha, maxAlpha);
    }
}
#endif
