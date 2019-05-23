//
//  VROBodyMesheriOS.cpp
//  ViroKit
//
//  Created by Raj Advani on 5/22/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROBodyMesheriOS.h"

#include "VROLog.h"
#include "VROTime.h"
#include "VROMath.h"
#include <mutex>
#include "VRODeviceUtil.h"
#include "VROGeometry.h"

#include "VROPoseFilterMovingAverage.h"
#include "VROPoseFilterLowPass.h"
#include "VROPoseFilterBoneDistance.h"
#include "VROPoseFilterEuro.h"
#include "VROOneEuroFilter.h"

#define BODYMESH 1

// Set to one of the above
#define VRO_BODY_MESHER_MODEL_A12 BODYMESH
#define VRO_BODY_MESHER_MODEL_A11 BODYMESH

#import "bodymesh.h"

static const float kConfidenceThreshold = 0.15;
static const float kInitialDampeningPeriodMs = 125;

#pragma mark - Initialization

VROBodyMesheriOS::VROBodyMesheriOS() {
    _isTracking = false;
}

VROBodyMesheriOS::~VROBodyMesheriOS() {
    
}

bool VROBodyMesheriOS::initBodyTracking(VROCameraPosition position,
                                         std::shared_ptr<VRODriver> driver) {
    
    MLModel *model;
    VRODeviceUtil *device = [[VRODeviceUtil alloc] init];
    bool A12 = [device isBionicA12];
    
    if (A12) {
#if VRO_BODY_MESHER_MODEL_A12==BODYMESH
        pinfo("Loading body meshing model");
        model = [[[bodymesh alloc] init] model];
#endif
    } else {
#if VRO_BODY_MESHER_MODEL_A11==BODYMESH
        pinfo("Loading body meshing model");
        model = [[[bodymesh alloc] init] model];
#endif
    }
    
    _visionEngine = std::make_shared<VROVisionEngine>(model, 224, position, VROCropAndScaleOption::Viro_RegionOfInterest);
    std::shared_ptr<VROVisionEngineDelegate> delegate = std::dynamic_pointer_cast<VROVisionEngineDelegate>(shared_from_this());
    passert (delegate != nullptr);
    _visionEngine->setDelegate(delegate);
    
    _poseFilter = std::make_shared<VROPoseFilterEuro>(kInitialDampeningPeriodMs, kConfidenceThreshold);
    return true;
}

void VROBodyMesheriOS::setDampeningPeriodMs(double period) {
    _dampeningPeriodMs = period;
    if (period <= 0) {
        _poseFilter = nullptr;
    } else {
        _poseFilter = std::make_shared<VROPoseFilterEuro>(_dampeningPeriodMs, kConfidenceThreshold);
    }
}

double VROBodyMesheriOS::getDampeningPeriodMs() const {
    return _dampeningPeriodMs;
}

void VROBodyMesheriOS::startBodyTracking() {
    _isTracking = true;
}

void VROBodyMesheriOS::stopBodyTracking() {
    _isTracking = false;
}

#pragma mark - Renderer Thread

void VROBodyMesheriOS::update(const VROARFrame *frame) {
    if (!_isTracking) {
        return;
    }
    _visionEngine->update(frame);
}

#pragma mark - Vision Queue (post-processing CoreML output)

// Invoked on the _visionQueue
std::vector<std::pair<VROVector3f, float>> VROBodyMesheriOS::processVisionOutput(VNCoreMLFeatureValueObservation *result, VROCameraPosition cameraPosition,
                                                                                  VROMatrix4f visionToImageSpace, VROMatrix4f imageToViewportSpace) {
    
    MLMultiArray *uvmap = result.featureValue.multiArrayValue;
    
    // TODO Make this allocation efficient again
    std::vector<std::pair<VROVector3f, float>> imageSpaceJoints(kNumBodyJoints);
    buildMesh(uvmap, cameraPosition, visionToImageSpace, imageToViewportSpace, imageSpaceJoints.data());
    
    std::weak_ptr<VROBodyMesheriOS> tracker_w = std::dynamic_pointer_cast<VROBodyMesheriOS>(shared_from_this());
    
    dispatch_async(dispatch_get_main_queue(), ^{
        std::shared_ptr<VROBodyMesheriOS> tracker = tracker_w.lock();
        
        if (tracker && tracker->_isTracking) {
            std::shared_ptr<VROBodyMesherDelegate> delegate = _bodyMeshDelegate_w.lock();
            if (delegate) {
                delegate->onBodyMeshUpdated();
            }
        }
    });
    
    return imageSpaceJoints;
}

std::shared_ptr<VROGeometry> VROBodyMesheriOS::buildMesh(MLMultiArray *uvmap, VROCameraPosition cameraPosition,
                                                         VROMatrix4f visionToImageSpace, VROMatrix4f imageToViewportSpace,
                                                         std::pair<VROVector3f, float> *outImageSpaceJoints) {
    if (uvmap.shape.count < 3) {
        return {};
    }
    
    int numJoints = (int) uvmap.shape[0].integerValue;
    int heatmapHeight = (int) uvmap.shape[1].integerValue;
    int heatmapWidth = (int) uvmap.shape[2].integerValue;
    
    passert (uvmap.dataType == MLMultiArrayDataTypeFloat32);
    float *array = (float *) uvmap.dataPointer;
    int stride_c = (int) uvmap.strides[0].integerValue;
    int stride_h = (int) uvmap.strides[1].integerValue;
    
    return {};
}
