//
//  VROBodyTrackeriOS.cpp
//  ViroKit
//
//  Created by vik.advani on 9/4/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROBodyTrackeriOS.h"

#include "VROLog.h"
#include "VROTime.h"
#include "VROMath.h"
#include <mutex>
#include "VRODeviceUtil.h"

#include "VROPoseFilterMovingAverage.h"
#include "VROPoseFilterLowPass.h"
#include "VROPoseFilterBoneDistance.h"
#include "VROPoseFilterEuro.h"
#include "VROOneEuroFilter.h"

#define HOURGLASS_2_1 1
#define HOURGLASS_2_1_T_DS 2
#define HOURGLASS_2_1_T 3

// Set to one of the above
#define VRO_BODY_TRACKER_MODEL_A12 HOURGLASS_2_1_T
#define VRO_BODY_TRACKER_MODEL_A11 HOURGLASS_2_1_DS_T

#import "hourglass_2_1.h"
#import "hourglass_2_1_t.h"
#import "hourglass_2_1_ds_t.h"

static const float kConfidenceThreshold = 0.15;
static const float kInitialDampeningPeriodMs = 125;

std::map<int, VROBodyJointType> _mpiiTypesToJointTypes = {
    { 0, VROBodyJointType::RightAnkle },
    { 1, VROBodyJointType::RightKnee },
    { 2, VROBodyJointType::RightHip },
    { 3, VROBodyJointType::LeftHip },
    { 4, VROBodyJointType::LeftKnee },
    { 5, VROBodyJointType::LeftAnkle },
    { 6, VROBodyJointType::Pelvis },
    { 7, VROBodyJointType::Thorax },
    { 8, VROBodyJointType::Neck },
    { 9, VROBodyJointType::Top },
    { 10, VROBodyJointType::RightWrist },
    { 11, VROBodyJointType::RightElbow },
    { 12, VROBodyJointType::RightShoulder },
    { 13, VROBodyJointType::LeftShoulder },
    { 14, VROBodyJointType::LeftElbow },
    { 15, VROBodyJointType::LeftWrist },
};

std::map<int, VROBodyJointType> _pilTypesToJointTypes = {
    { 0, VROBodyJointType::Top },
    { 1, VROBodyJointType::Neck },
    { 2, VROBodyJointType::RightShoulder },
    { 3, VROBodyJointType::RightElbow },
    { 4, VROBodyJointType::RightWrist },
    { 5, VROBodyJointType::LeftShoulder },
    { 6, VROBodyJointType::LeftElbow },
    { 7, VROBodyJointType::LeftWrist },
    { 8, VROBodyJointType::RightHip },
    { 9, VROBodyJointType::RightKnee },
    { 10, VROBodyJointType::RightAnkle },
    { 11, VROBodyJointType::LeftHip },
    { 12, VROBodyJointType::LeftKnee },
    { 13, VROBodyJointType::LeftAnkle },
    { 14, VROBodyJointType::Thorax },
    { 15, VROBodyJointType::Pelvis },
};

#pragma mark - Initialization

VROBodyTrackeriOS::VROBodyTrackeriOS() {
    _isTracking = false;
}

VROBodyTrackeriOS::~VROBodyTrackeriOS() {
   
}

bool VROBodyTrackeriOS::initBodyTracking(VROCameraPosition position,
                                         std::shared_ptr<VRODriver> driver) {

    MLModel *model;
    VRODeviceUtil *device = [[VRODeviceUtil alloc] init];
    bool A12 = [device isBionicA12];
    
    if (A12) {
#if VRO_BODY_TRACKER_MODEL_A12==HOURGLASS_2_1
        pinfo("Loading HG_2-1 body tracking model");
        model = [[[hourglass_2_1 alloc] init] model];
#elif VRO_BODY_TRACKER_MODEL_A12==HOURGLASS_2_1_T
        pinfo("Loading HG_2-1-T body tracking model");
        model = [[[hourglass_2_1_t alloc] init] model];
#endif
    } else {
#if VRO_BODY_TRACKER_MODEL_A11==HOURGLASS_2_1
        pinfo("Loading HG_2-1 body tracking model");
        model = [[[hourglass_2_1 alloc] init] model];
#elif VRO_BODY_TRACKER_MODEL_A11==HOURGLASS_2_1_T
        pinfo("Loading HG_2-1-T body tracking model");
        model = [[[hourglass_2_1_t alloc] init] model];
#elif VRO_BODY_TRACKER_MODEL_A11==HOURGLASS_2_1_DS_T
        pinfo("Loading HG_2-1-T-ds body tracking model");
        model = [[[hourglass_2_1_ds_t alloc] init] model];
#endif
    }
    
    _visionEngine = std::make_shared<VROVisionEngine>(model, position, VROCropAndScaleOption::Viro_RegionOfInterest);
    std::shared_ptr<VROVisionEngineDelegate> delegate = std::dynamic_pointer_cast<VROVisionEngineDelegate>(shared_from_this());
    passert (delegate != nullptr);
    _visionEngine->setDelegate(delegate);
    
    _poseFilter = std::make_shared<VROPoseFilterEuro>(kInitialDampeningPeriodMs, kConfidenceThreshold);
    return true;
}

void VROBodyTrackeriOS::setDampeningPeriodMs(double period) {
    _dampeningPeriodMs = period;
    if (period <= 0) {
        _poseFilter = nullptr;
    } else {
        _poseFilter = std::make_shared<VROPoseFilterEuro>(_dampeningPeriodMs, kConfidenceThreshold);
    }
}

double VROBodyTrackeriOS::getDampeningPeriodMs() const {
    return _dampeningPeriodMs;
}

void VROBodyTrackeriOS::startBodyTracking() {
    _isTracking = true;
}

void VROBodyTrackeriOS::stopBodyTracking() {
    _isTracking = false;
}

#pragma mark - Renderer Thread

void VROBodyTrackeriOS::update(const VROARFrame *frame) {
    if (!_isTracking) {
        return;
    }
    _visionEngine->update(frame);
}

#pragma mark - Vision Queue (post-processing CoreML output)

// Invoked on the _visionQueue
std::vector<std::pair<VROVector3f, float>> VROBodyTrackeriOS::processVisionOutput(VNCoreMLFeatureValueObservation *result, VROCameraPosition cameraPosition,
                                                                                  VROMatrix4f visionToImageSpace, VROMatrix4f imageToViewportSpace) {
    
    MLMultiArray *heatmap = result.featureValue.multiArrayValue;
    
    // TODO Make this allocation efficient again
    std::vector<std::pair<VROVector3f, float>> imageSpaceJoints(kNumBodyJoints);
    VROPoseFrame joints = convertHeatmap(heatmap, cameraPosition, visionToImageSpace, imageToViewportSpace, imageSpaceJoints.data());

    std::weak_ptr<VROBodyTrackeriOS> tracker_w = std::dynamic_pointer_cast<VROBodyTrackeriOS>(shared_from_this());

    if (!joints.empty()) {
        dispatch_async(dispatch_get_main_queue(), ^{
            std::shared_ptr<VROBodyTrackeriOS> tracker = tracker_w.lock();
            
            if (tracker && tracker->_isTracking) {
                std::shared_ptr<VROBodyTrackerDelegate> delegate = _bodyMeshDelegate_w.lock();
                if (delegate) {
                    if (!tracker->_poseFilter) {
                        delegate->onBodyJointsFound(joints);
                    } else {
                        delegate->onBodyJointsFound(tracker->_poseFilter->filterJoints(joints));
                    }
                }
            }
        });
    }
    
    return imageSpaceJoints;
}

VROPoseFrame VROBodyTrackeriOS::convertHeatmap(MLMultiArray *heatmap, VROCameraPosition cameraPosition,
                                               VROMatrix4f visionToImageSpace, VROMatrix4f imageToViewportSpace,
                                               std::pair<VROVector3f, float> *outImageSpaceJoints) {
    if (heatmap.shape.count < 3) {
        return {};
    }
    
    int numJoints = (int) heatmap.shape[0].integerValue;
    int heatmapHeight = (int) heatmap.shape[1].integerValue;
    int heatmapWidth = (int) heatmap.shape[2].integerValue;
    
    passert (heatmap.dataType == MLMultiArrayDataTypeFloat32);
    float *array = (float *) heatmap.dataPointer;
    int stride_c = (int) heatmap.strides[0].integerValue;
    int stride_h = (int) heatmap.strides[1].integerValue;
    
    VROInferredBodyJoint bodyMap[kNumBodyJoints];
    double creationTime = VROTimeCurrentMillis();
    
    /*
     The ML model will return the heatmap tiles for each joint; choose the highest
     confidence tile for each joint.
     */
    for (int k = 0; k < numJoints; k++) {
        VROBodyJointType type = _mpiiTypesToJointTypes[k];
        if (type == VROBodyJointType::Unknown) {
            continue;
        }
        
        for (int i = 0; i < heatmapHeight; i++) {
            for (int j = 0; j < heatmapWidth; j++) {
                long index = k * stride_c + i * stride_h + j;
                float confidence = array[index];
                
                if (confidence > 0) {
                    VROInferredBodyJoint &joint = bodyMap[(int) type];
                    
                    /*
                     The point we create here is just the index of the heatmap tile
                     (i and j). We will convert this into a floating point value once
                     we find the highest confidence tile.
                     */
                    if (confidence > joint.getConfidence()) {
                        VROInferredBodyJoint inferredJoint(type);
                        inferredJoint.setConfidence(confidence);
                        inferredJoint.setTileIndices(j, i);
                        inferredJoint.setCreationTime(creationTime);
                        
                        bodyMap[(int) type] = inferredJoint;
                    }
                }
            }
        }
    }
    
    /*
     Now we have a map with the highest confidence tile for each joint. Convert the
     heatmap tile indices into normalized coordinates [0, 1].
     */
    for (VROInferredBodyJoint &joint : bodyMap) {
        if (joint.getConfidence() > 0) {
            VROVector3f    tilePoint  = { (float) joint.getTileX(), (float) joint.getTileY() };
            
            // Convert tile indices to vision coordinates [0, 1]
            VROVector3f visionPoint = { (tilePoint.x + 0.5f) / (float) (heatmapWidth),
                (tilePoint.y + 0.5f) / (float) (heatmapHeight), 0 };
            
            // Multiply by the ARKit transform to get normalized viewport coordinates [0, 1]
            VROVector3f imagePoint = visionToImageSpace.multiply(visionPoint);
            VROVector3f viewportPoint = imageToViewportSpace.multiply(imagePoint);
            
            // Store the image points for bounding box computation
            outImageSpaceJoints[(int) joint.getType()].first = imagePoint;
            outImageSpaceJoints[(int) joint.getType()].second = joint.getConfidence();
            
            // Mirror the X dimension if we're using the front-facing camera
            if (cameraPosition == VROCameraPosition::Front) {
                viewportPoint.x = 1.0 - viewportPoint.x;
            }
            VROBoundingBox viewportBounds = VROBoundingBox(viewportPoint.x, viewportPoint.x, viewportPoint.y, viewportPoint.y, 0, 0);
            joint.setBounds(viewportBounds);
        }
    }
    
    VROPoseFrame poseFrame = newPoseFrame();
    for (int i = 0; i < kNumBodyJoints; i++) {
        VROInferredBodyJoint &inferredJoint = bodyMap[i];
        if (inferredJoint.getConfidence() > 0) {
            poseFrame[i].push_back(inferredJoint);
        }
    }
    return poseFrame;
}
