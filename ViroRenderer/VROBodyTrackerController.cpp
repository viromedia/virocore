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
#include "VROBox.h"
#include "VROProjector.h"
#include "VROBillboardConstraint.h"
#include "VROTime.h"
#include "VROARFrame.h"

#if VRO_PLATFORM_IOS
#include "VRODriverOpenGLiOS.h"
#include "VROAnimBodyDataiOS.h"
#endif

static const float kAutomaticSizingRatio = 1;
static const float kSkeletonTorsoHeight = 0.640863955;
static const VROVector3f kInitialModelPos = VROVector3f(-10, -10, 10);
static const float kUsePresetDepthDistanceMeter = 1;
static const VROBodyJointType kArHitTestJoint = VROBodyJointType::Neck;

VROBodyTrackerController::VROBodyTrackerController(std::shared_ptr<VRORenderer> renderer,
                                                   std::shared_ptr<VRODriver> driver,
                                                   std::shared_ptr<VRONode> sceneRoot) {
    _currentTrackedState = VROBodyTrackedState::NotAvailable;
    _calibrating = false;
    _renderer = renderer;
    
    _bodyControllerRoot = std::make_shared<VRONode>();
    sceneRoot->addChildNode(_bodyControllerRoot);

#if VRO_PLATFORM_IOS
    _view = (VROViewAR *) std::dynamic_pointer_cast<VRODriverOpenGLiOS>(driver)->getView();
#endif
}

VROBodyTrackerController::~VROBodyTrackerController() {
}

bool VROBodyTrackerController::bindModel(std::shared_ptr<VRONode> modelRootNode) {
    _bodyControllerRoot->removeAllChildren();
    _modelRootNode = nullptr;
    
    _modelRootNode = modelRootNode;
    _bodyControllerRoot->setScale(VROVector3f(1, 1, 1));
    return true;
}

void VROBodyTrackerController::startCalibration() {
    if (_calibrating) {
        return;
    }

    // Clear previously calibrated data.
    _calibrating = true;
    _projectedPlanePosition = kInitialModelPos;
    _projectedPlaneNormal = VROVector3f(0, 0, 0);

    // Reset the model and bones back to it's initial configuration
    std::shared_ptr<VRONode> parentNode = _modelRootNode->getParentNode();
    _modelRootNode->setScale(VROVector3f(1, 1, 1));
    _modelRootNode->setRotation(VROQuaternion());
    _modelRootNode->setPosition(kInitialModelPos);
    _modelRootNode->removeAllConstraints();
    _modelRootNode->computeTransforms(parentNode->getWorldTransform(), parentNode->getWorldRotation());
}

void VROBodyTrackerController::onBodyJointsFound(const VROPoseFrame &inferredJoints) {
    if (_modelRootNode == nullptr) {
        return;
    }

    // Convert to VROBodyJoint data structure, using only first joint of each type
    std::map<VROBodyJointType, VROBodyJoint> joints;
    for (int i = 0; i < kNumBodyJoints; i++) {
        auto &kv = inferredJoints[i];
        if (!kv.empty()) {
            VROInferredBodyJoint inferred = kv[0];
            
            VROBodyJoint joint = { inferred.getType(), inferred.getConfidence() };
            joint.setScreenCoords({ inferred.getBounds().getX(), inferred.getBounds().getY(), 0 });
            joint.setSpawnTimeMs(VROTimeCurrentMillis());
            joints[(VROBodyJointType) i] = joint;
        }
    }

    // Filter new joints found given by the VROBodyTracker and update _cachedTrackedJoints
    processJoints(joints);
    
    // Ensure we at least have the root ml joint before updating our model (neck)
    if (_currentTrackedState != NotAvailable) {
        // Only update the model if we have the required scalable joints (hips)
        if (_currentTrackedState == LimitedEffectors || _currentTrackedState == FullEffectors) {
            // Reset the model and bones back to it's initial configuration
            std::shared_ptr<VRONode> parentNode = _modelRootNode->getParentNode();
            _modelRootNode->setScale(VROVector3f(1, 1, 1));
            _modelRootNode->setRotation(VROQuaternion());
            _modelRootNode->setPosition(kInitialModelPos);
            _modelRootNode->computeTransforms(parentNode->getWorldTransform(),
                                              parentNode->getWorldRotation());
            
            // Dynamically scale the model to the right size.
            calibrateModelToMLTorsoScale();
        }
        
        // Only calibrate the rig with the results if we haven't yet done so.
        if (_calibrating) {
            // If we are calibrating without scale joints, we may not have found
            // the hips yet. Set a reasonable scale for now.
            if (_currentTrackedState == NoScalableJointsAvailable) {
                float fixedUserTorsoHeight = 0.45; // Average torso height.
                float modelToMLRatio = fixedUserTorsoHeight / kSkeletonTorsoHeight * kAutomaticSizingRatio;
                _modelRootNode->setScale(VROVector3f(modelToMLRatio, modelToMLRatio, modelToMLRatio));
            }
            
            std::shared_ptr<VROBodyTrackerControllerDelegate> delegate = _delegate.lock();
            if (delegate) {
                delegate->onCalibrationFinished();
            }
            _calibrating = false;
        }
    }

    // Always notify our delegates with the latest set of joint data.
    notifyOnJointUpdateDelegates();
}

void VROBodyTrackerController::notifyOnJointUpdateDelegates() {
    std::shared_ptr<VROBodyTrackerControllerDelegate> delegate = _delegate.lock();
    if (delegate == nullptr) {
        return;
    }

    // Construct a map containing filtered cached ML joints before dampening.
    std::map<VROBodyJointType, VROBodyTrackerControllerDelegate::VROJointPos> jointsFiltered;
    for (auto &jointPair : _cachedTrackedJoints) {
        VROBodyJoint bodyJoint = jointPair.second;

        VROBodyTrackerControllerDelegate::VROJointPos filteredJoint;
        filteredJoint.screenPosX = bodyJoint.getScreenCoords().x;
        filteredJoint.screenPosY = bodyJoint.getScreenCoords().y;
        filteredJoint.worldPosition = bodyJoint.getProjectedTransform().extractTranslation();
        
        jointsFiltered[jointPair.first] = filteredJoint;
    }

    // Construct a map containing dampened ML positional joint data.
    std::map<VROBodyJointType, VROVector3f> jointsDampened;
    for (auto &mlJoint : _cachedModelJoints) {
        jointsDampened[mlJoint.first] = mlJoint.second;
    }

    delegate->onJointUpdate(jointsFiltered, jointsDampened, {});
}

void VROBodyTrackerController::processJoints(const std::map<VROBodyJointType, VROBodyJoint> &joints) {
    // Grab all the 2D joints of high confidence for the targets we want.
    std::map<VROBodyJointType, VROBodyJoint> latestJoints;
    for (auto &kv : joints) {
        latestJoints[kv.first] = kv.second;
    }

    // First, convert the joints into 3d space.
    projectJointsInto3DSpace(latestJoints);

    // Then, perform filtering and update our known set of cached joints.
    updateCachedJoints(latestJoints);
    
    // With the new found joints, update the current tracking state
    bool hasRequiredJoints = true;
    
    // First examine if we have the joints needed for positioning and scaling.
    if (_cachedTrackedJoints.find(VROBodyJointType::Neck) == _cachedTrackedJoints.end()) {
        hasRequiredJoints = false;
    }
    
    // Then, examine if we have the joints needed for scaling (right + left) or (Top) joints.
    bool hasHipJoints = true;
    if (_cachedTrackedJoints.find(VROBodyJointType::RightHip) == _cachedTrackedJoints.end() ||
        _cachedTrackedJoints.find(VROBodyJointType::LeftHip) == _cachedTrackedJoints.end())  {
        hasHipJoints = false;
    }
    if (!hasRequiredJoints) {
        setBodyTrackedState(VROBodyTrackedState::NotAvailable);
    } else if (!hasHipJoints) {
        setBodyTrackedState(VROBodyTrackedState::NoScalableJointsAvailable);
    } else if (_cachedTrackedJoints.size() >= 4) {
        setBodyTrackedState(VROBodyTrackedState::LimitedEffectors);
    }

    // Finally update _cachedModelJoints to be set on the IKRig.
    _cachedModelJoints.clear();
    
    // If we are not dampening, simply set the _cachedModelJoints and return.
    for (auto &jointPair : _cachedTrackedJoints) {
        VROVector3f pos = jointPair.second.getProjectedTransform().extractTranslation();
        _cachedModelJoints[jointPair.first] = pos;
    }
}

void VROBodyTrackerController::projectJointsInto3DSpace(std::map<VROBodyJointType, VROBodyJoint> &latestJoints) {
    if (latestJoints.find(kArHitTestJoint) == latestJoints.end()) {
        return;
    }

    // Always project the depth to a known position in space.
    VROVector3f camPos = _renderer->getCamera().getPosition();
    VROVector3f camForward = _renderer->getCamera().getForward();
    VROVector3f finalPos = camPos + (camForward * kUsePresetDepthDistanceMeter);

    _projectedPlanePosition = finalPos;
    _projectedPlaneNormal = (camPos - _projectedPlanePosition).normalize();

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
    _cachedTrackedJoints.clear();
    for (auto &latestjointPair : latestJoints) {
        _cachedTrackedJoints[latestjointPair.first] = latestjointPair.second;
    }
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

void VROBodyTrackerController::calibrateModelToMLTorsoScale() {
    // Calculate the neckToMLRoot distance
    VROVector3f neckPos = _cachedModelJoints[VROBodyJointType::Neck];
    VROVector3f midHipLoc = getMLRootPosition();
    float userTorsoHeight = midHipLoc.distanceAccurate(neckPos);
    
    // Calculate the different distances, grab the ratio.
    float modelToMLRatio = userTorsoHeight / kSkeletonTorsoHeight * kAutomaticSizingRatio;
    
    // Apply that ratio to the scale of the model.
    _modelRootNode->setScale(VROVector3f(modelToMLRatio, modelToMLRatio, modelToMLRatio));
}

VROVector3f VROBodyTrackerController::getMLRootPosition() {
    if (_currentTrackedState == VROBodyTrackedState::NotAvailable) {
        pwarn("Unable to determine ML Root position without proper body tracking data.");
        return VROVector3f();
    }
    
    VROVector3f start = _cachedModelJoints[VROBodyJointType::LeftHip];
    VROVector3f end = _cachedModelJoints[VROBodyJointType::RightHip];
    return (start - end).scale(0.5f) + end;
}
