//
//  VROBodyTrackerController.h
//  ViroSample
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROBodyTrackerController_h
#define VROBodyTrackerController_h

#include "VROARDeclarativeNode.h"
#include "VROSceneController.h"
#include "VRODefines.h"
#include "VROBodyTracker.h"
#include "VROEventDelegate.h"
#include "VRORenderer.h"
#if VRO_PLATFORM_IOS
#include "VROViewAR.h"
#import <UIKit/UIKit.h>
#endif

/*
 States representing the current tracking fidelity of this VROBodyTrackerController.
 */
enum VROBodyTrackedState {
    // Root body part effector required for body tracking is not found.
    NotAvailable,

    // Limited tracking where root and some body effectors are found, but not all.
    LimitedEffectors,

    // Successfully achieved full tracking for all root and body effectors.
    FullEffectors,
};

/*
 Joint that is tracked by this controller.
 */
class VROBodyJoint {
public:
    VROBodyJoint() : _type(VROBodyJointType::Top), _confidence(0) {}
    VROBodyJoint(VROBodyJointType type, double confidence) :
        _type(type),
        _confidence(confidence),
        _projectedTransform(VROMatrix4f::identity()),
        _hasValidProjectedTransform(false){}
    
    const VROVector3f &getScreenCoords() const {
        return _screenCoords;
    }
    void setScreenCoords(VROVector3f screenCoords) {
        _screenCoords = screenCoords;
    }
    
    const VROMatrix4f &getProjectedTransform() const {
        return _projectedTransform;
    }
    void setProjectedTransform(VROMatrix4f transform) {
        _projectedTransform = transform;
        _hasValidProjectedTransform = true;
    }
    void clearPojectedTransform() {
        _projectedTransform.toIdentity();
        _hasValidProjectedTransform = false;
    }
    bool hasValidProjectedTransform() const {
        return _hasValidProjectedTransform;
    }
    
    void setSpawnTimeMs(double ts) {
        _spawnTimeMs = ts;
    }
    double getSpawnTimeMs() const {
        return _spawnTimeMs;
    }
    
    VROBodyJointType getType() const {
        return _type;
    }
    
    double getConfidence() const {
        return _confidence;
    }
    
private:
    VROVector3f _screenCoords;
    VROMatrix4f _projectedTransform;
    bool _hasValidProjectedTransform;
    VROBodyJointType _type;
    double _confidence;
    double _spawnTimeMs;
};

/*
 Delegate set on a VROBodyTrackerController for notifying listeners about VROBodyTrackedState updates.
 */
class VROBodyTrackerControllerDelegate {
public:
    VROBodyTrackerControllerDelegate() {};
    virtual ~VROBodyTrackerControllerDelegate() {};

    /*
     Triggered when the VROBodyTrackedState for the attached VROBodyTrackerController has changed.
     */
    virtual void onBodyTrackStateUpdate(VROBodyTrackedState state) = 0;
};

/*
 VROBodyTrackerController coordinates the filtering, projecting and feeding of body tracking data
 from the VROBodyTracker into the currently bounded 3D model's VROIKRig for driving body motion.
 */
class VROBodyTrackerController : public VROBodyTrackerDelegate,
                                 public std::enable_shared_from_this<VROBodyTrackerController> {
public:

    VROBodyTrackerController(std::shared_ptr<VRORenderer> renderer, std::shared_ptr<VRONode> sceneRoot);
    ~VROBodyTrackerController();

    /*
     True if the given 3D model has been successfully bounded to this controller.
     */
    bool bindModel(std::shared_ptr<VRONode> modelRootNode);

    /*
     Notifies the controller to start aligning the underlying 3D model's root with
     the latest ML joint data.
     TODO VIRO-4674: Remove Manual Calibration
     */
    void startCalibration();

    /*
     Notifies the controller to finish calibration and initialize the VROIKRig
     and align it with the latest ML joint data.
     TODO VIRO-4674: Remove Manual Calibration
     */
    void finishCalibration();

    /*
     Sets a VROBodyTrackerControllerDelegate on this controller for
     onBodyTrackStateUpdate() notifications.
     */
    void setDelegate(std::shared_ptr<VROBodyTrackerControllerDelegate> delegate);

    // VROBodyTrackerDelegate
    void onBodyJointsFound(const std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> &joints);

    /*
     Sets the window period at which we sample points for dampening. If period == 0,
     no dampening will be applied.
     */
    void setDampeningPeriodMs(double period);
#if VRO_PLATFORM_IOS
    void enableDebugMLViewIOS(std::shared_ptr<VRODriver> driver);
    void updateDebugMLViewIOS(const std::map<VROBodyJointType, VROBodyJoint> &joints);
#endif

private:
    /*
     Set renderer needed for performing Ar Hit tests.
     */
    std::shared_ptr<VRORenderer> _renderer;

    /*
     The current VROBodyTrackedState of this controller.
     */
    VROBodyTrackedState _currentTrackedState;

    /*
     Set VROBodyTrackerControllerDelegate for notifying listeners about VROBodyTrackedState updates.
     */
    std::weak_ptr<VROBodyTrackerControllerDelegate> _delegate;

    /*
     A cache of all filtered ML joints provided by the VROBodyTracker thus far with a
     valid position in 3D space.
     */
    std::map<VROBodyJointType, VROBodyJoint> _cachedTrackedJoints;

    /*
     Final filtered and processed joint positional data on which to apply onto the IKRig.
     */
    std::map<VROBodyJointType, VROVector3f> _cachedModelJoints;

    /*
     Period at which to sample ML joint data upon for dampening.
     */
    double _dampeningPeriodMs;

    /*
     A window dampening data for which to perform SMA / EMA analysis.
     */
    std::map<VROBodyJointType, std::vector<std::pair<double, VROVector3f>>> _cachedJointWindow;

    /*
     A cache of all known effector's last known position in reference to the root in world space.
     */
    std::map<VROBodyJointType, VROMatrix4f> _cachedEffectorRootOffsets;

    /*
     The root position and normal of the plane to project onto when using kProjectToPlaneTracking
     depth tests.
     */
    VROVector3f _projectedPlanePosition;
    VROVector3f _projectedPlaneNormal;

    /*
     The rig, skinner and node associated with the currently bound model.
     */
    std::shared_ptr<VROIKRig> _rig;
    std::shared_ptr<VROSkinner> _skinner;
    std::shared_ptr<VRONode> _modelRootNode;

    /*
     Map of pre-set keys to bone IDs within the 3D model for this rig.
     */
    std::map<std::string, int> _keyToEffectorMap;

    /*
     Map of ML joints to bone IDs within the 3D model for this rig.
     */
    std::map<VROBodyJointType, int> _mlJointForBoneIndex;

    /*
     Map of ML Joints and it's corresponding timeout periods when filtering joint data.
     */
    std::map<VROBodyJointType, double> _mlJointTimeoutMap;

    /*
     Saved transform from the ML Neck position to the IKRoot of this model during calibration.
     */
    VROMatrix4f _mlRootToModelRoot;

    /*
     Saved Neck to Hip distance, used for calculating automatic torso resizing ratios.
     */
    float _originalNeckToHipDistance;

    /*
     True if this controller is currently calibrating the latest set of ML joints to the IKRig.
     */
    bool _calibrating;
    std::shared_ptr<VROEventDelegate> _calibrationEventDelegate;

    /*
     Debugging UI components containing debug box nodes representing the locations of
     tracked ML body positions.
     */
    std::shared_ptr<VRONode> _bodyControllerRoot;
    std::map<VROBodyJointType, std::shared_ptr<VRONode>> _debugBoxEffectors;
    std::shared_ptr<VRONode> _debugBoxRoot;
#if VRO_PLATFORM_IOS
    // iOS UI Components
    UILabel *_labelViews[14];
    UIView *_bodyViews[14];
    VROViewAR *_view;
#endif

    /*
     Process, filter and update this controller's latest known set of _cachedTrackedJoints
     with the latest found ML 2D points given by VROBodyTracker.
     */
    void processJoints(const std::map<VROBodyJointType, VROBodyJoint> &joints);
    void projectJointsInto3DSpace(std::map<VROBodyJointType, VROBodyJoint> &joints);
    void updateCachedJoints(std::map<VROBodyJointType, VROBodyJoint> &joints);
    void restoreMissingJoints(std::vector<VROBodyJoint> expiredJoints);
    void dampenCachedJoints();

    /*
     Updates the current VROBodyTrackedState and notifies the attached VROBodyTrackerControllerDelegate.
     */
    void setBodyTrackedState(VROBodyTrackedState state);

    /*
     Updates 3D model's rig with the latest set of known 3D positions
     */
    void updateModel();

    /*
     Align the 3D model's root position / root motion to the root ML joint.
     */
    void alignModelRootToMLRoot();

    /*
     Returns true if the given targetTransform is reachable form the given parent's joint
     considering it's bone's length.
     */
    bool isTargetReachableFromParentBone(VROBodyJoint targetJoint);

    /*
     Depth tests for projecting a 2D ML screen coordinate into 3D space.
     */
    bool performDepthTest(float x, float y, VROMatrix4f &matOut);
    bool performWindowDepthTest(float x, float y, VROMatrix4f &matOut);
    bool performUnprojectionToPlane(float x, float y, VROMatrix4f &matOut);

    /*
     Initializes the model's uniform scale needed for automatic resizing.
     */
    void initializeModelUniformScale();

    /*
     Called during the calibration phase to scale the model's torso uniformly to
     fit the 3D model to body joints found by VROBodytracker.
     */
    void alignModelTorsoScale();

    /*
     Debug UI used by this controller.
     */
    std::shared_ptr<VRONode> createDebugBoxUI(bool isAffector, std::string tag);
};

/*
 Delegate used for handling events to calibrate this VROBodyTracker.
 TODO VIRO-4674: Remove Manual Calibration
 */
class VROBodyTrackerControllerEventDelegate : public VROEventDelegate {
public:
    VROBodyTrackerControllerEventDelegate(std::shared_ptr<VROBodyTrackerController> controller) {
        _controller = controller;
    }
    virtual ~VROBodyTrackerControllerEventDelegate() {};

    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position) {
        if (clickState == ClickState::ClickUp){
            std::shared_ptr<VROBodyTrackerController> controller = _controller.lock();
            if (!controller) {
                return;
            }

            controller->finishCalibration();
        }
    }

    void onPinch(int source, std::shared_ptr<VRONode> node, float scaleFactor, PinchState pinchState) {
        std::shared_ptr<VROBodyTrackerController> controller = _controller.lock();
        if (!controller) {
            return;
        }

        if(pinchState == PinchState::PinchStart) {
            _scaleStart = node->getScale().x; // xyz is in uniform scale.
        } else {
             node->setScale(VROVector3f(_scaleStart, _scaleStart, _scaleStart).scale(scaleFactor));
        }
    }

private:
    float _scaleStart;
    std::weak_ptr<VROBodyTrackerController> _controller;
};
#endif /* VROBodyTrackerController_h  */
