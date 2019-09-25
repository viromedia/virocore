//
//  VROBodyTrackerController.cpp
//  ViroSample
//
//  Copyright © 2018 Viro Media. All rights reserved.
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
#include "VROSkeletonRenderer.h"
#endif

static const float kAutomaticSizingRatio = 1;
static const float kSkeletonTorsoHeight = 0.640863955;
static const VROVector3f kInitialModelPos = VROVector3f(-10, -10, 10);
static const float kUsePresetDepthDistanceMeter = 1;

VROBodyTrackerController::VROBodyTrackerController(std::shared_ptr<VRORenderer> renderer,
                                                   std::shared_ptr<VRODriver> driver,
                                                   std::shared_ptr<VROBodyTracker> tracker,
                                                   std::shared_ptr<VRONode> sceneRoot) {
    _currentTrackedState = VROBodyTrackedState::NotAvailable;
    _needsInitialCalibration = false;
    _renderer = renderer;
    _drawSkeleton = false;
    
    _bodyControllerRoot = std::make_shared<VRONode>();
    sceneRoot->addChildNode(_bodyControllerRoot);

#if VRO_PLATFORM_IOS
    _view = (VROViewAR *) std::dynamic_pointer_cast<VRODriverOpenGLiOS>(driver)->getView();
    if (tracker) {
        _skeletonRenderer = std::make_shared<VROSkeletonRenderer>(_view, tracker);
    }
#endif
}

VROBodyTrackerController::~VROBodyTrackerController() {
}

void VROBodyTrackerController::setDelegate(std::shared_ptr<VROBodyTrackerControllerDelegate> delegate) {
    _delegate = delegate;
}

void VROBodyTrackerController::setDrawSkeleton(bool drawSkeleton) {
    _drawSkeleton = drawSkeleton;
}

bool VROBodyTrackerController::bindModel(std::shared_ptr<VRONode> modelRootNode) {
    _bodyControllerRoot->removeAllChildren();
    _modelRootNode = nullptr;
    
    _modelRootNode = modelRootNode;
    _bodyControllerRoot->setScale(VROVector3f(1, 1, 1));
    return true;
}

#pragma mark - Joint Processing

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

    projectJointsInto3DSpace(joints);
    updateBodyTrackingState(joints);
    
    // Update calibration
    std::map<VROBodyJointType, VROBodyTrackerControllerDelegate::VROJointPosition> jointPositions = extractJointPositions(joints);
    updateCalibration(jointPositions);

    // Always notify our delegates with the latest set of joint data    
    std::shared_ptr<VROBodyTrackerControllerDelegate> delegate = _delegate.lock();
    if (delegate) {
        delegate->onJointUpdate(jointPositions);
    }
#if VRO_PLATFORM_IOS
    if (_skeletonRenderer && _drawSkeleton) {
        _skeletonRenderer->onBodyJointsFound(inferredJoints);
    }
#endif
}

std::map<VROBodyJointType, VROBodyTrackerControllerDelegate::VROJointPosition> VROBodyTrackerController::extractJointPositions(const std::map<VROBodyJointType, VROBodyJoint> &latestJoints) {
    
    std::map<VROBodyJointType, VROBodyTrackerControllerDelegate::VROJointPosition> jointPositions;
    for (auto &jointPair : latestJoints) {
        VROBodyJoint bodyJoint = jointPair.second;

        VROBodyTrackerControllerDelegate::VROJointPosition joint;
        joint.screenPosX = bodyJoint.getScreenCoords().x;
        joint.screenPosY = bodyJoint.getScreenCoords().y;
        joint.worldPosition = bodyJoint.getProjectedTransform().extractTranslation();
        
        jointPositions[jointPair.first] = joint;
    }
    return jointPositions;
}

#pragma mark - Point Projection

void VROBodyTrackerController::projectJointsInto3DSpace(std::map<VROBodyJointType, VROBodyJoint> &joints) {
    // Always project the depth to a known position in space.
    VROVector3f camPos = _renderer->getCamera().getPosition();
    VROVector3f camForward = _renderer->getCamera().getForward();
    VROVector3f finalPos = camPos + (camForward * kUsePresetDepthDistanceMeter);

    _projectedPlanePosition = finalPos;
    _projectedPlaneNormal = (camPos - _projectedPlanePosition).normalize();

    // Project the 2D joints into 3D coordinates as usual.
    for (auto &joint : joints) {
        VROMatrix4f hitTransform;
        float pointX = joint.second.getScreenCoords().x;
        float pointY = joint.second.getScreenCoords().y;
        bool success = performUnprojectionToPlane(pointX, pointY, &hitTransform);
        if (!success) {
            joint.second.clearProjectedTransform();
        } else {
            joint.second.setProjectedTransform(hitTransform);
        }
    }

    // Remove points that have failed projections from the map of latestJoints
    for (auto it = joints.cbegin(), next_it = it; it != joints.cend(); it = next_it) {
        ++next_it;
        if (!it->second.hasValidProjectedTransform()) {
            joints.erase(it);
        }
    }
}

bool VROBodyTrackerController::performUnprojectionToPlane(float x, float y, VROMatrix4f *outMatrix) {
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
    
    outMatrix->toIdentity();
    outMatrix->translate(intersectionPoint);
    return success;
}

#pragma mark - Body Tracking State

void VROBodyTrackerController::updateBodyTrackingState(const std::map<VROBodyJointType, VROBodyJoint> &joints) {
    bool hasHipJoints = joints.find(VROBodyJointType::RightHip) != joints.end() &&
                        joints.find(VROBodyJointType::LeftHip) != joints.end();
    
    if (joints.empty()) {
        setBodyTrackingState(VROBodyTrackedState::NotAvailable);
    } else if (!hasHipJoints) {
        setBodyTrackingState(VROBodyTrackedState::NoScalableJointsAvailable);
    } else if (joints.size() == kNumBodyJoints) {
        setBodyTrackingState(VROBodyTrackedState::FullEffectors);
    } else if (joints.size() >= 4) {
        setBodyTrackingState(VROBodyTrackedState::LimitedEffectors);
    }
}

void VROBodyTrackerController::setBodyTrackingState(VROBodyTrackedState state) {
    if (_currentTrackedState == state) {
        return;
    }
    
    _currentTrackedState = state;
    std::shared_ptr<VROBodyTrackerControllerDelegate> delegate = _delegate.lock();
    if (delegate != nullptr) {
        delegate->onBodyTrackStateUpdate(_currentTrackedState);
    }
}

#pragma mark - Calibration

void VROBodyTrackerController::startCalibration() {
    if (_needsInitialCalibration) {
        return;
    }
    
    // Clear previously calibrated data
    _needsInitialCalibration = true;
    _projectedPlanePosition = kInitialModelPos;
    _projectedPlaneNormal = VROVector3f(0, 0, 0);
    
    // Reset the model and bones back to their initial configuration
    std::shared_ptr<VRONode> parentNode = _modelRootNode->getParentNode();
    _modelRootNode->setScale(VROVector3f(1, 1, 1));
    _modelRootNode->setRotation(VROQuaternion());
    _modelRootNode->setPosition(kInitialModelPos);
    _modelRootNode->removeAllConstraints();
    _modelRootNode->computeTransforms(parentNode->getWorldTransform(), parentNode->getWorldRotation());
}

void VROBodyTrackerController::updateCalibration(const std::map<VROBodyJointType, VROBodyTrackerControllerDelegate::VROJointPosition> &joints) {
    // Every frame we update the calibration scale using the latest joints
    if (_currentTrackedState != NotAvailable) {
        if (_currentTrackedState == LimitedEffectors || _currentTrackedState == FullEffectors) {
            // Reset the model and bones back to their initial configuration
            std::shared_ptr<VRONode> parentNode = _modelRootNode->getParentNode();
            _modelRootNode->setScale(VROVector3f(1, 1, 1));
            _modelRootNode->setRotation(VROQuaternion());
            _modelRootNode->setPosition(kInitialModelPos);
            _modelRootNode->computeTransforms(parentNode->getWorldTransform(),
                                              parentNode->getWorldRotation());
            
            // Dynamically scale the model to the right size
            calibrateModelToMLTorsoScale(joints);
        }
        
        // If we're waiting for our first calibration...
        if (_needsInitialCalibration) {
            // If we didn't get enough joint data, make the first calibration use
            // a sensible default
            if (_currentTrackedState == NoScalableJointsAvailable) {
                float fixedUserTorsoHeight = 0.45; // Average torso height.
                float modelToMLRatio = fixedUserTorsoHeight / kSkeletonTorsoHeight * kAutomaticSizingRatio;
                _modelRootNode->setScale(VROVector3f(modelToMLRatio, modelToMLRatio, modelToMLRatio));
            }
            
            std::shared_ptr<VROBodyTrackerControllerDelegate> delegate = _delegate.lock();
            if (delegate) {
                delegate->onCalibrationFinished();
            }
            _needsInitialCalibration = false;
        }
    }
}

void VROBodyTrackerController::calibrateModelToMLTorsoScale(const std::map<VROBodyJointType, VROBodyTrackerControllerDelegate::VROJointPosition> &joints) const {
    auto neck = joints.find(VROBodyJointType::Neck);
    if (neck == joints.end()) {
        return;
    }
    
    // Calculate the neckToMLRoot distance
    VROVector3f neckPos = neck->second.worldPosition;
    VROVector3f midHipLoc = getMLRootPosition(joints);
    float userTorsoHeight = midHipLoc.distanceAccurate(neckPos);
    
    // Calculate the different distances, grab the ratio.
    float modelToMLRatio = userTorsoHeight / kSkeletonTorsoHeight * kAutomaticSizingRatio;
    
    // Apply that ratio to the scale of the model.
    _modelRootNode->setScale(VROVector3f(modelToMLRatio, modelToMLRatio, modelToMLRatio));
}

VROVector3f VROBodyTrackerController::getMLRootPosition(const std::map<VROBodyJointType, VROBodyTrackerControllerDelegate::VROJointPosition> &joints) const {
    if (_currentTrackedState == VROBodyTrackedState::NotAvailable) {
        pwarn("Unable to determine ML Root position without proper body tracking data.");
        return VROVector3f();
    }
    
    auto leftHip = joints.find(VROBodyJointType::LeftHip);
    auto rightHip = joints.find(VROBodyJointType::RightHip);
    
    if (leftHip != joints.end() && rightHip != joints.end()) {
        VROVector3f start = leftHip->second.worldPosition;
        VROVector3f end   = rightHip->second.worldPosition;
        return (start - end).scale(0.5f) + end;
    } else {
        pwarn("Unable to determine ML Root position without proper body tracking data.");
        return VROVector3f();
    }
}
