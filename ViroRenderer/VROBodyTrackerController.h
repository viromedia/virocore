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
#include "VROBodyPlayer.h"
#include "VROEventDelegate.h"
#include "VRORenderer.h"
#include "VROBodyAnimData.h"

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
    /*
     Represents the positional ML joint data in world space and its corresponding
     2D screen space position.
     */
    struct VROJointPos {
        VROVector3f worldPosition;
        float screenPosX;
        float screenPosY;
    };

    VROBodyTrackerControllerDelegate() {};
    virtual ~VROBodyTrackerControllerDelegate() {};

    /*
     Triggered when the calibration finishes
     */
    virtual void onCalibrationFinished() = 0;
    
    /*
     Triggered when the VROBodyTrackedState for the attached VROBodyTrackerController has changed.
     */
    virtual void onBodyTrackStateUpdate(VROBodyTrackedState state) = 0;

    /*
     Triggered when the controller has processed new joints after it has been calibrated.

     TODO: Remove unnecessary joint maps after narrowing down which joint data to use and expose.
     */
    virtual void onJointUpdate(const std::map<VROBodyJointType, VROJointPos> &mlJointsFiltered,
                               const std::map<VROBodyJointType, VROVector3f> &mlJointsDampened,
                               const std::map<VROBodyJointType, VROMatrix4f> &modelJoints) = 0;
};

/*
 Body specific transforms and data attained from a successful VROBodyTrackerController calibration.
 Used to bind new models without requiring recalibration.
 */
struct VROBodyCalibratedConfig {
    float torsoLength;
    VROVector3f projectedPlanePosition;
    VROVector3f projectedPlaneNormal;
    std::map<std::string, float> _mlBoneLengths;
    std::map<std::string, float> _modelBoneLengths;
};

/*
 VROBodyTrackerController coordinates the filtering, projecting and feeding of body tracking data
 from the VROBodyTracker into the currently bounded 3D model's VROIKRig for driving body motion.
 */
class VROBodyTrackerController : public VROBodyTrackerDelegate, public VROBodyPlayerDelegate,
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
    void startCalibration(bool manual = false);

    /*
     Notifies the controller to finish calibration and initialize the VROIKRig
     and align it with the latest ML joint data.
     TODO VIRO-4674: Remove Manual Calibration
     */
    void finishCalibration(bool manual = false);
    void calibrateRigWithResults();

    /*
     Returns the currently set or last calibrated VROBodyCalibratedConfig on this controller.
     */
    std::shared_ptr<VROBodyCalibratedConfig> getCalibratedConfiguration();

    /*
     Automatically calibrates the currently bounded 3D model with the given VROBodyCalibratedConfig.
     */
    void setCalibratedConfiguration(std::shared_ptr<VROBodyCalibratedConfig> config);

    /*
     Sets a VROBodyTrackerControllerDelegate on this controller for
     onBodyTrackStateUpdate() notifications.
     */
    void setDelegate(std::shared_ptr<VROBodyTrackerControllerDelegate> delegate);

    // VROBodyTrackerDelegate
    void onBodyJointsFound(const VROPoseFrame &joints);

    // VROBodyPlaybackDelegate
    void onBodyJointsPlayback(const std::map<VROBodyJointType, VROVector3f> &joints, VROBodyPlayerStatus status);

    void onBodyPlaybackStarting(std::shared_ptr<VROBodyAnimData> animData);

    /*
     Debug flag to show / hide debug cubes demonstrating the joint locations within
     this controller.
     */
    void setDisplayDebugCubes(bool visible);
    bool getDisplayDebugCubes();

    /*
     Sets the staleness threshold for the given joint in milliseconds that joints
     data are checked against before being evicted from the cache.
     */
    void setStalenessThresholdForJoint(VROBodyJointType type, float timeoutMs);
    float getStalenessThresholdForJoint(VROBodyJointType type);

#if VRO_PLATFORM_IOS
    void enableDebugMLViewIOS(std::shared_ptr<VRODriver> driver);
    void updateDebugMLViewIOS(const std::map<VROBodyJointType, VROBodyJoint> &joints);
#endif

    /*
     Start recording the body tracking session. Invoke stop recording to get a JSON String of recording
     tracking values.
     */
    void startRecording();

    /*
      Stop recording the body tracking session. Must be invoked after startRecording.
     */
    std::string stopRecording();
    bool isRecording() { return _isRecording;};

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
     A cache of all known effector's last known position in reference to the root in world space.
     */
    std::map<VROBodyJointType, VROMatrix4f> _cachedEffectorRootOffsets;

    /*
     The root position and normal of the plane to project onto when using kProjectToPlaneTracking
     depth tests.
     */
    std::vector<VROVector3f> _candidatePlanePositions;
    VROVector3f _projectedPlanePosition;
    VROVector3f _projectedPlaneNormal;

    /*
     The rig, skeleton and node associated with the currently bound model.
     */
    std::shared_ptr<VROIKRig> _rig;
    std::shared_ptr<VROSkeleton> _skeleton;
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
    float _skeletonTorsoHeight;
    float _userTorsoHeight;

    /*
     True if this controller is currently calibrating the latest set of ML joints to the IKRig.
     */
    bool _calibrating;
    bool _shouldCalibrateRigWithResults;
    std::shared_ptr<VROEventDelegate> _calibrationEventDelegate;
    std::shared_ptr<VROBodyCalibratedConfig> _calibratedConfiguration;

    /*
     Used to restore any pre-set event delegate on the model's node after setting the
     calibration event delegate.
     */
    std::shared_ptr<VROEventDelegate> _preservedEventDelegate;

    /*
     Debugging UI components containing debug box nodes representing the locations of
     tracked ML body positions.
     */
    std::shared_ptr<VRONode> _bodyControllerRoot;
    std::map<VROBodyJointType, std::shared_ptr<VRONode>> _debugBoxEffectors;
    std::shared_ptr<VRONode> _debugBoxRoot;

    std::shared_ptr<VROBodyAnimDataRecorder> _bodyAnimDataRecorder;
                                     
    // Set to true to begin recording the body tracking. Default is false.
    bool _isRecording;

    // the time in milliseconds when recording of body tracking started.
    double _startRecordingTime;

    // the time in milliseconds when recording of body tracking stopped.
    double _endRecordingTime;
    VROMatrix4f _initRecordWorldTransformOfRootNode;

    // Matrix representing the start root world transform of the model when playback occurs.
    VROMatrix4f _playbackRootStartMatrix;

    // Multiple all playback joints through below. Below is equal _playbackRootStartMatrix.inverse() * _playbackDataStartMatrix;
    VROMatrix4f _playbackDataFinalTransformMatrix;
    
    // Anim data recorder. 
    std::shared_ptr<VROBodyAnimDataRecorder> _animDataRecorder;

    /*
     Configurable filter thresholds and debug switches.
     */
    bool _displayDebugCubes;

#if VRO_PLATFORM_IOS
    // iOS UI Components
    UILabel *_labelViews[16];
    UIView *_bodyViews[16];
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

    /*
     Updates the current VROBodyTrackedState and notifies the attached VROBodyTrackerControllerDelegate.
     */
    void setBodyTrackedState(VROBodyTrackedState state);

    /*
     Updates 3D model's rig with the latest set of known 3D positions
     */
    void updateModel();
    void notifyOnJointUpdateDelegates();

    /*
     Align the 3D model's root position / root motion to the root ML joint.
     */
    void alignModelRootToMLRoot();

    /*
     Depth tests for projecting a 2D ML screen coordinate into 3D space.
     */
    bool findTorsoClusteredDepth(std::map<VROBodyJointType, VROBodyJoint> &latestJoints,
                                 VROMatrix4f &matOut);
    VROVector3f findClusterInPoints(std::vector<VROVector3f> points);
    bool performDepthTest(float x, float y, VROMatrix4f &matOut);
    bool performWindowDepthTest(float x, float y, VROMatrix4f &matOut);
    bool performUnprojectionToPlane(float x, float y, VROMatrix4f &matOut);
    void findDampenedTorsoClusteredDepth(std::map<VROBodyJointType, VROBodyJoint> &latestJoints);

    /*
     Initializes the model's uniform scale needed for automatic resizing.
     */
    void calculateSkeletonTorsoDistance();

    /*
     Calculates an mlJointToModelRoot offset transform between 'root' mlJoint bones
     in the skeleton and _modelRootNode.
     */
    void calibrateMlToModelRootOffset();
    /*
     Called during the calibration phase to scale the model's torso uniformly to
     fit the 3D model to body joints found by VROBodytracker.
     */
    void calibrateModelToMLTorsoScale();

    /*
     The ML root position as referenced by the VROBodyTrackerController.
     */
    VROVector3f getMLRootPosition();

    /*
     Debug UI used by this controller.
     */
    std::shared_ptr<VRONode> createDebugBoxUI(bool isAffector, std::string tag);

    /*
     Map of bone names and the length leading up to them.
     */
    std::map<std::string, float> _mlBoneLengths;
    std::map<std::string, float> _modelBoneLengths;

    /*
     Restores the Top bone's geometry bind transform of the model.
     */
    void restoreTopBoneTransform();

    /*
     Examines the mlToModel bone scaling ratio and modifies the model's skeletal
     rig to reflect that visually. This calibration can only be done with a
     _currentTrackedState of FullEffectors.
     */
    void calibrateBoneProportionality();

    /*
     Iterates through every VROBodyJointType, calculates intermediary bone
     distances between them and stores the results in _mlBoneLengths and
     _modelBoneLengths.
     */
    void calculateKnownBoneSizes(VROBodyJointType joint);

    /*
     Manually infers the transformations and bone lengths of unknown joint types.
     This is mainly done on torso bones that were not detected as a part of the
     ML body tracker.
     */
    void calculateInferredBoneSizes();

    /*
     Calculates the length from the previous bone transform to the targetBone and
     updates our bone length maps with the results.
     */
    void updateBoneLengthReference(VROVector3f previousModelBoneTrans,
                                   VROVector3f previousMlBoneTrans, VROBodyJointType targetBone);

    /*
     Scales the model's skeletal bone transforms proportionally, knowing the ratio
     between the ml bone lengths, and model bone lengths.
     */
    void scaleBoneTransform(std::string childBone, std::string parentBone, VROVector3f scaleDir);

    /*
     Return true if this BodyController is ignoring the given joint.
     */
    bool isIgnoredJoint(VROBodyJointType joint);
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

            //controller->finishCalibration();
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
