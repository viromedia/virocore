//
//  VROARImageTrackerOutput.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#if ENABLE_OPENCV

#ifndef VROARImageTrackerOutput_h
#define VROARImageTrackerOutput_h

#include <memory>
#include "opencv2/core/core.hpp"
#include "VROARImageTarget.h"

class VROARImageTrackerOutput {
public:
    bool found;
    std::vector<cv::Point2f> corners;
    cv::Mat translation;
    cv::Mat rotation;
    std::shared_ptr<VROARImageTarget> target;
    
    // TODO: remove this?
    cv::Mat outputImage;

    static std::shared_ptr<VROARImageTrackerOutput> createFalseOutput();
    VROARImageTrackerOutput(bool found);
    VROARImageTrackerOutput(bool found, std::vector<cv::Point2f> corners, cv::Mat translation, cv::Mat rotation, std::shared_ptr<VROARImageTarget> target);
    ~VROARImageTrackerOutput();
    
private:
    
};

#endif /* VROARImageTrackerOutput_hpp */

#endif /* ENABLE_OPENCV */
