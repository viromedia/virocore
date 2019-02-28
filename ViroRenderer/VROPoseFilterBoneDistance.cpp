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
    
    VROPoseFrame filteredFrame = newPoseFrame();
    for (int i = 0; i < kNumBodyJoints; i++) {
        VROBodyJointType type = (VROBodyJointType) i;
        
        // For knee joints, spatial filter by ensuring the X distance from hip to knee is
        // less than some multiple of the Y distance from hip to ankle. This eliminates
        // knee noise (and knees are generally harder for the ML model to identify). Note
        // this may also eliminate some peculiar Yoga poses where the knee is brought
        // very near to the hip.
        if (type == VROBodyJointType::LeftKnee) {
            VROVector3f hipPosition   = getJointPosition(newFrame, VROBodyJointType::LeftHip);
            VROVector3f kneePosition  = getJointPosition(newFrame, VROBodyJointType::LeftKnee);
            VROVector3f anklePosition = getJointPosition(newFrame, VROBodyJointType::LeftAnkle);
            
            if (!hipPosition.isZero() && !kneePosition.isZero() && !anklePosition.isZero()) {
                float distanceHipKneeX = fabs(hipPosition.x - kneePosition.x);
                float distanceHipAnkleY = fabs(hipPosition.y - anklePosition.y);
                
                if (distanceHipKneeX < distanceHipAnkleY * kKneeAnomalyThreshold) {
                    filteredFrame[i] = newFrame[i];
                }
            }
        }
        else if (type == VROBodyJointType::RightKnee) {
            VROVector3f hipPosition   = getJointPosition(newFrame, VROBodyJointType::RightHip);
            VROVector3f kneePosition  = getJointPosition(newFrame, VROBodyJointType::RightKnee);
            VROVector3f anklePosition = getJointPosition(newFrame, VROBodyJointType::RightAnkle);
            
            if (!hipPosition.isZero() && !kneePosition.isZero() && !anklePosition.isZero()) {
                float distanceHipKneeX = fabs(hipPosition.x - kneePosition.x);
                float distanceHipAnkleY = fabs(hipPosition.y - anklePosition.y);
                
                if (distanceHipKneeX < distanceHipAnkleY * kKneeAnomalyThreshold) {
                    filteredFrame[i] = newFrame[i];
                }
            }
        }
        else {
            filteredFrame[i] = newFrame[i];
        }
    }

    return filteredFrame;
}

VROPoseFrame VROPoseFilterBoneDistance::temporalFilter(const std::vector<VROPoseFrame> &frames, const VROPoseFrame &combinedFrame,
                                                       const VROPoseFrame &newFrame) {
    VROPoseFrame filteredFrame = newPoseFrame();
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
    
    for (int i = 0; i < kNumBodyJoints; i++) {
        if (!discarded[i]) {
            filteredFrame[i] = newFrame[i];
        }
    }
    return filteredFrame;
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
    auto &samplesA = frame[(int) jointA];
    auto &samplesB = frame[(int) jointB];
    
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

VROVector3f VROPoseFilterBoneDistance::getJointPosition(const VROPoseFrame &frame, VROBodyJointType joint) {
    auto &samples = frame[(int) joint];
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
    auto &samplesA = frame[(int) jointA];
    if (samplesA.empty()) {
        return {};
    }
    auto &samplesB = frame[(int) jointB];
    if (samplesB.empty()) {
        return {};
    }
    return (samplesB.front().getCenter() - samplesA.front().getCenter()).normalize();
}

