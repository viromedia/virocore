//
//  VROARImageTrackerOutput.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARImageTrackerOutput_h
#define VROARImageTrackerOutput_h

#include <memory>
#include "opencv2/core/core.hpp"

class VROARImageTrackerOutput {
public:
    bool found;
    std::vector<cv::Point2f> corners;
    cv::Mat translation;
    cv::Mat rotation;
    
    // TODO: remove this?
    cv::Mat outputImage;

    static std::shared_ptr<VROARImageTrackerOutput> createFalseOutput();
    VROARImageTrackerOutput(bool found);
    VROARImageTrackerOutput(bool found, std::vector<cv::Point2f> corners, cv::Mat translation, cv::Mat rotation);
    ~VROARImageTrackerOutput();
    
private:
    
};

#endif /* VROARImageTrackerOutput_hpp */
