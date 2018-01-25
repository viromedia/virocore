//
//  VROImageTracker.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROImageTracker_h
#define VROImageTracker_h

#include "VROLog.h"
#include "VROImageTrackerOutput.h"
#include "opencv2/core/core.hpp"
#include <memory>

enum class VROImageTrackerType {
    BRISK,
    ORB,
    ORB3,
    ORB4,
};

class VROImageTracker {
public:
    static std::shared_ptr<VROImageTracker> createImageTracker(cv::Mat image);

    // Constructor, assumes the targetImage is in RGB format.
    VROImageTracker(cv::Mat targetImage, VROImageTrackerType type);

    // Sets the tracker type we want to use
    void setType(VROImageTrackerType type);

    // This function takes the input image and detects/computes the keypoints and descriptors (as return arguments)
    void detectKeypointsAndDescriptors(cv::Mat inputImage,
                                       std::vector<cv::KeyPoint> &keypoints,
                                       cv::Mat &descriptors);

    // Finds the _targetImage in the inputImage (assumes RGB format).
    std::shared_ptr<VROImageTrackerOutput> findTarget(cv::Mat inputImage);
    std::shared_ptr<VROImageTrackerOutput> findTarget(cv::Mat inputImage, float* intrinsics);
    std::shared_ptr<VROImageTrackerOutput> findTarget(std::vector<cv::KeyPoint> inputKeypoints,
                                                      cv::Mat inputDescriptors, cv::Mat inputImage);

private:
    
    void updateTargetInfo();

    std::shared_ptr<VROImageTrackerOutput> findTargetInternal(cv::Mat inputImage);
    std::shared_ptr<VROImageTrackerOutput> findTargetBF(std::vector<cv::KeyPoint> inputKeypoints,
                                                        cv::Mat inputDescriptors,  cv::Mat inputImage);
    
    void testSolvePnP();
    
    long getCurrentTimeMs();
    long _startTime;

    cv::Mat _targetImage;
    VROImageTrackerType _type;
    int _matcherType;
    std::vector<cv::KeyPoint> _targetKeyPoints;
    cv::Mat _targetDescriptors;
    
    cv::Mat _translation;
    cv::Mat _rotation;

    float *_intrinsics;
};

#endif /* VROImageTracker_h */
