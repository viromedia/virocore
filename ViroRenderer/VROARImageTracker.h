//
//  VROARImageTracker.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#if ENABLE_OPENCV

#ifndef VROARImageTracker_h
#define VROARImageTracker_h

#include "VROLog.h"
#include "VROARImageTrackerOutput.h"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <memory>

enum class VROARImageTrackerType {
    BRISK,
    ORB,
    ORB3,
    ORB4,
};

class VROARImageTracker {
public:
    static std::shared_ptr<VROARImageTracker> createARImageTracker(cv::Mat image);

    // Constructor, assumes the targetImage is in RGB format.
    VROARImageTracker(cv::Mat targetImage, VROARImageTrackerType type);

    // Sets the tracker type we want to use
    void setType(VROARImageTrackerType type);

    // This function takes the input image and detects/computes the keypoints and descriptors (as return arguments)
    void detectKeypointsAndDescriptors(cv::Mat inputImage,
                                       std::vector<cv::KeyPoint> &keypoints,
                                       cv::Mat &descriptors);

    // Finds the _targetImage in the inputImage (assumes RGB format).
    std::shared_ptr<VROARImageTrackerOutput> findTarget(cv::Mat inputImage);
    std::shared_ptr<VROARImageTrackerOutput> findTarget(cv::Mat inputImage, float* intrinsics);
    std::shared_ptr<VROARImageTrackerOutput> findTarget(std::vector<cv::KeyPoint> inputKeypoints,
                                                        cv::Mat inputDescriptors, cv::Mat inputImage);

private:
    
    void updateTargetInfo();

    std::shared_ptr<VROARImageTrackerOutput> findTargetInternal(cv::Mat inputImage);
    std::shared_ptr<VROARImageTrackerOutput> findTargetBF(std::vector<cv::KeyPoint> inputKeypoints,
                                                          cv::Mat inputDescriptors,  cv::Mat inputImage);
    
    void testSolvePnP();
    
    long getCurrentTimeMs();
    long _startTime;

    cv::Mat _targetImage;
    VROARImageTrackerType _type;
    cv::Ptr<cv::Feature2D> _feature;
    int _matcherType;
    std::vector<cv::KeyPoint> _targetKeyPoints;
    cv::Mat _targetDescriptors;
    
    cv::Mat _translation;
    cv::Mat _rotation;

    float *_intrinsics;
};

#endif /* VROARImageTracker_h */

#endif /* ENABLE_OPENCV */
