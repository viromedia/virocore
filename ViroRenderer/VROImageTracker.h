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
#include <memory>
#include "opencv2/core/core.hpp"

class VROImageTracker {
public:
    static std::shared_ptr<VROImageTracker> createImageTracker(cv::Mat image);
    VROImageTracker(cv::Mat targetImage);

    std::shared_ptr<VROImageTrackerOutput> findTarget(cv::Mat inputImage);
private:
    void detectKeypointsAndDescriptors(cv::Mat inputImage,
                                       std::vector<cv::KeyPoint> &keypoints,
                                       cv::Mat &descriptors);

    cv::Mat _targetImage;
    std::vector<cv::KeyPoint> _targetKeyPoints;
    cv::Mat _targetDescriptors;
};

#endif /* VROImageTracker_h */
