//
//  VROPoseFilterBoneDistance.cpp
//  ViroKit
//
//  Created by Raj Advani on 2/27/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROPoseFilterBoneDistance.h"
#include "VROLog.h"
#include "VROMath.h"
#include <set>
#include <algorithm>

static const float kKneeAnomalyThreshold = 0.75;
static const float kAnomalyThreshold = 0.5f;
static const float kAnomalyDegrees = 45;

std::vector<std::pair<int, int>> kPoseFilterSkeleton = {{0, 1}, {1, 5}, {5, 6}, {6, 7}, {1, 2},
    {2, 3}, {3, 4}, {1, 14}, {14, 15}, {15, 11}, {11, 12}, {12, 13}, {15, 8}, {8, 9}, {9, 10}};

VROPoseFrame VROPoseFilterBoneDistance::spatialFilter(const std::vector<VROPoseFrame> &pastFrames, const VROPoseFrame &combinedFrame,
                                                      const VROPoseFrame &newFrame) {
    
    VROPoseFrame filteredFrame;
    for (auto kv : newFrame) {
        VROBodyJointType type = kv.first;
        
        // For knee joints, spatial filter by ensuring the X distance from hip to knee is
        // less than some multiple of the Y distance from hip to ankle. This eliminates
        // knee noise (and knees are generally harder for the ML model to identify). Note
        // this may also eliminate some peculiar Yoga poses where the knee is brought
        // very near to the hip.
        if (type == VROBodyJointType::LeftKnee) {
            VROVector3f hipPosition   = getPosition(newFrame, VROBodyJointType::LeftHip);
            VROVector3f kneePosition  = getPosition(newFrame, VROBodyJointType::LeftKnee);
            VROVector3f anklePosition = getPosition(newFrame, VROBodyJointType::LeftAnkle);
            
            if (!hipPosition.isZero() && !kneePosition.isZero() && !anklePosition.isZero()) {
                float distanceHipKneeX = fabs(hipPosition.x - kneePosition.x);
                float distanceHipAnkleY = fabs(hipPosition.y - anklePosition.y);
                
                if (distanceHipKneeX < distanceHipAnkleY * kKneeAnomalyThreshold) {
                    filteredFrame[type] = kv.second;
                }
            }
        }
        else if (type == VROBodyJointType::RightKnee) {
            VROVector3f hipPosition   = getPosition(newFrame, VROBodyJointType::RightHip);
            VROVector3f kneePosition  = getPosition(newFrame, VROBodyJointType::RightKnee);
            VROVector3f anklePosition = getPosition(newFrame, VROBodyJointType::RightAnkle);
            
            if (!hipPosition.isZero() && !kneePosition.isZero() && !anklePosition.isZero()) {
                float distanceHipKneeX = fabs(hipPosition.x - kneePosition.x);
                float distanceHipAnkleY = fabs(hipPosition.y - anklePosition.y);
                
                if (distanceHipKneeX < distanceHipAnkleY * kKneeAnomalyThreshold) {
                    filteredFrame[type] = kv.second;
                }
            }
        }
        else {
            filteredFrame[type] = kv.second;
        }
    }

    return filteredFrame;
}

VROPoseFrame VROPoseFilterBoneDistance::temporalFilter(const std::vector<VROPoseFrame> &frames, const VROPoseFrame &combinedFrame,
                                                       const VROPoseFrame &newFrame) {
    VROPoseFrame filteredFrame;
    std::vector<bool> discarded(kNumBodyJoints, false);
    
    for (std::pair<int, int> limb : kPoseFilterSkeleton) {
        VROBodyJointType jointA = (VROBodyJointType) limb.first;
        VROBodyJointType jointB = (VROBodyJointType) limb.second;
        
        // If we're already discarded a joint from this limb, ignore and move to the next limb
        // e.g. if we discarded the Neck joint when checking Head-->Neck, then don't check Neck-->Thorax.
        if (discarded[(int) jointA] || discarded[(int) jointB]) {
            continue;
        }
        
        float averageLimbLength = getAverageLimbLength(frames, jointA, jointB);
        float currentLimbLength = getLimbLength(newFrame, jointA, jointB);
        
        VROVector3f averageLimbDirection = getAverageLimbDirection(frames, jointA, jointB);
        VROVector3f currentLimbDirection = getLimbDirection(newFrame, jointA, jointB);
        
        float averageLimbAngle = toDegrees(averageLimbDirection.angleWithVector({0, 1, 0}));
        float currentLimbAngle = toDegrees(currentLimbDirection.angleWithVector({0, 1, 0}));

        if (currentLimbLength < averageLimbLength * (1 - kAnomalyThreshold) ||
            currentLimbLength > averageLimbLength * (1 + kAnomalyThreshold) ||
            currentLimbAngle < (averageLimbAngle - kAnomalyDegrees) ||
            currentLimbAngle > (averageLimbAngle + kAnomalyDegrees)) {
            
            discarded[(int)jointB] = true;
            
            // Keep debug info for now
            // if (jointB == VROBodyJointType::RightKnee) {
            //    pinfo("Discarded Right knee average limb length %f, current %f, average angle %f, current %f", averageLimbLength, currentLimbLength, averageLimbAngle, currentLimbAngle);
            //    auto j = newFrame.find(jointB);
            //    if (j != newFrame.end() && !j->second.empty()) {
            // if (j->second[0].getTileX() < 14) {
            //            pinfo("    Right knee anomaly");
            //       }
            //   }
            // }
        }
    }
    
    for (auto &kv : newFrame) {
        if (!discarded[(int) kv.first]) {
            filteredFrame[kv.first] = newFrame.find(kv.first)->second;
        }
    }
    return filteredFrame;
}

void VROPoseFilterBoneDistance::addJoints(const VROPoseFrame &joints, VROBodyJointType jointA, VROBodyJointType jointB,
                                          VROPoseFrame *result) {
    (*result)[jointA] = joints.find(jointA)->second;
    (*result)[jointB] = joints.find(jointB)->second;
}

void VROPoseFilterBoneDistance::addNonAnomalousJoints(const VROPoseFrame &frame, const VROPoseFrame &joints,
                                                      VROBodyJointType jointA, VROBodyJointType jointB, VROPoseFrame *result) {
    VROBodyJointType anomalous = getAnomalousJoint(frame, joints, jointA, jointB);
    if (anomalous == jointA) {
        (*result)[jointB] = joints.find(jointB)->second;
    } else if (anomalous == jointB) {
        (*result)[jointA] = joints.find(jointA)->second;
    } else {
        (*result)[jointA] = joints.find(jointA)->second;
        (*result)[jointB] = joints.find(jointB)->second;
    }
}

VROBodyJointType VROPoseFilterBoneDistance::getAnomalousJoint(const VROPoseFrame &frame, const VROPoseFrame &joints,
                                                              VROBodyJointType jointA, VROBodyJointType jointB) {

    float distanceA = getDistanceFromPriors(frame, joints, jointA);
    float distanceB = getDistanceFromPriors(frame, joints, jointB);
    
    if (distanceA > 0 && distanceA > distanceB) {
        return jointA;
    } else if (distanceB > 0 && distanceB > distanceA) {
        return jointB;
    } else {
        return VROBodyJointType::Unknown;
    }
}

float VROPoseFilterBoneDistance::getDistanceFromPriors(const VROPoseFrame &frame, const VROPoseFrame &joints, VROBodyJointType type) {
    auto previousIt = frame.find(type);
    auto currentIt = joints.find(type);
    
    if (previousIt != frame.end() && currentIt != joints.end() && !currentIt->second.empty()) {
        VROVector3f averagePosition = getPosition(frame, type);
        if (!averagePosition.isZero()) {
            return averagePosition.distance(currentIt->second[0].getCenter());
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

float VROPoseFilterBoneDistance::getAverageLimbLength(const std::vector<VROPoseFrame> &frames,
                                                      VROBodyJointType jointA, VROBodyJointType jointB) {
    float sumDistance = 0;
    int numFramesWithBothJoints = 0;
    
    for (const VROPoseFrame &frame : frames) {
        float length = getLimbLength(frame, jointA, jointB);
        if (length > 0) {
            sumDistance += length;
            numFramesWithBothJoints++;
        }
    }
    if (numFramesWithBothJoints == 0) {
        return 0;
    } else {
        return sumDistance / (float) numFramesWithBothJoints;
    }
}

float VROPoseFilterBoneDistance::getLimbLength(const VROPoseFrame &frame,
                                               VROBodyJointType jointA, VROBodyJointType jointB) {
    
    auto kvA = frame.find(jointA);
    if (kvA == frame.end()) {
        return 0;
    }
    auto kvB = frame.find(jointB);
    if (kvB == frame.end()) {
        return 0;
    }
    const std::vector<VROInferredBodyJoint> &samplesA = kvA->second;
    const std::vector<VROInferredBodyJoint> &samplesB = kvB->second;
    
    size_t numSamples = std::min(samplesA.size(), samplesB.size());
    if (numSamples == 0) {
        return 0;
    }
    
    float sumDistance = 0;
    for (size_t i = 0; i < numSamples; i++) {
        sumDistance += samplesA[i].getCenter().distance(samplesB[i].getCenter());
    }
    return sumDistance / (float) numSamples;
}

VROVector3f VROPoseFilterBoneDistance::getPosition(const VROPoseFrame &frame, VROBodyJointType joint) {
    auto kv = frame.find(joint);
    if (kv == frame.end()) {
        return {};
    }
    const std::vector<VROInferredBodyJoint> &samples = kv->second;
    if (samples.empty()) {
        return {};
    }
    
    VROVector3f sumPosition;
    for (const VROInferredBodyJoint &sample : samples) {
        sumPosition += sample.getCenter();
    }
    return sumPosition / (float) samples.size();
}

VROVector3f VROPoseFilterBoneDistance::getAverageLimbDirection(const std::vector<VROPoseFrame> &frames,
                                                         VROBodyJointType jointA, VROBodyJointType jointB) {
    VROVector3f sumDirections;
    int numFramesWithBothJoints = 0;
    
    for (const VROPoseFrame &frame : frames) {
        VROVector3f direction = getLimbDirection(frame, jointA, jointB);
        if (!direction.isZero()) {
            sumDirections += direction;
            numFramesWithBothJoints++;
        }
    }
    if (numFramesWithBothJoints == 0) {
        return {};
    } else {
        return sumDirections / (float) numFramesWithBothJoints;
    }
}

VROVector3f VROPoseFilterBoneDistance::getLimbDirection(const VROPoseFrame &frame, VROBodyJointType jointA, VROBodyJointType jointB) {
    auto kvA = frame.find(jointA);
    if (kvA == frame.end() || kvA->second.empty()) {
        return {};
    }
    auto kvB = frame.find(jointB);
    if (kvB == frame.end() || kvB->second.empty()) {
        return {};
    }
    return (kvB->second.front().getCenter() - kvA->second.front().getCenter()).normalize();
}

