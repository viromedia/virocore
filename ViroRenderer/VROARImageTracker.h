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
#include "VROARImageTarget.h"
#include <memory>
#include <map>

enum class VROARImageTrackerType {
    BRISK,
    ORB,
    ORB3,
    ORB4,
};

struct VROARImageTargetOpenCV {
    std::shared_ptr<VROARImageTarget> arImageTarget;
    std::vector<cv::KeyPoint> keyPoints;
    cv::Mat descriptors;
    cv::Mat rotation; // the most recent found rotation for this target in the last input image
    cv::Mat translation; // the most recent found translation for this target in the last input image
};

// TODO: merge this class into VROARTrackingSession
class VROARImageTracker {
public:
    // helper static function for "legacy"/older tracking test code w/ only 1 target
    static std::shared_ptr<VROARImageTracker> createARImageTracker(std::shared_ptr<VROARImageTarget> arImageTarget);
    
    // helper static function to create a tracker with the best configuration.
    static std::shared_ptr<VROARImageTracker> createDefaultTracker();

    VROARImageTracker(VROARImageTrackerType type);

    // Adds/Removes ARImageTarget from tracking
    void addARImageTarget(std::shared_ptr<VROARImageTarget> arImageTarget);
    void removeARImageTarget(std::shared_ptr<VROARImageTarget> arImageTarget);

    // Sets the tracker type we want to use
    void setType(VROARImageTrackerType type);

    // This function takes the input image and detects/computes the keypoints and descriptors (as return arguments)
    void detectKeypointsAndDescriptors(cv::Mat inputImage,
                                       std::vector<cv::KeyPoint> &keypoints,
                                       cv::Mat &descriptors);

    /*
     Finds the _arImageTarget in the inputImage (assumes RGB format). Uses the given intrinsics matrix. Pass in
     NULL for intrinsics ptr if one doesn't exist.
     */
    std::vector<std::shared_ptr<VROARImageTrackerOutput>> findTarget(cv::Mat inputImage, float* intrinsics);

private:
    
    void updateType();
    VROARImageTargetOpenCV updateTargetInfo(std::shared_ptr<VROARImageTarget> arImageTarget);

    std::vector<std::shared_ptr<VROARImageTrackerOutput>> findTargetInternal(cv::Mat inputImage);
    
    std::vector<std::shared_ptr<VROARImageTrackerOutput>> findMultipleTargetsBF(std::vector<cv::KeyPoint> inputKeypoints,
                                                                                cv::Mat inputDescriptors,  cv::Mat inputImage);
    
    long getCurrentTimeMs();
    long _startTime;

    std::shared_ptr<VROARImageTarget> _arImageTarget;

    VROARImageTrackerType _type;
    cv::Ptr<cv::Feature2D> _feature;
    int _matcherType;

    std::vector<std::shared_ptr<VROARImageTarget>> _arImageTargets;
    std::map<std::shared_ptr<VROARImageTarget>, VROARImageTargetOpenCV> _targetsMap;

    // TODO: remove, used by the "single" target tracker
//    std::vector<cv::KeyPoint> _targetKeyPoints;
//    cv::Mat _targetDescriptors;
//
//    cv::Mat _translation;
//    cv::Mat _rotation;

    float *_intrinsics;
};

#endif /* VROARImageTracker_h */

#endif /* ENABLE_OPENCV */
