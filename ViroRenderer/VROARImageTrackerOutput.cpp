//
//  VROARImageTrackerOutput.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARImageTrackerOutput.h"

std::shared_ptr<VROARImageTrackerOutput> VROARImageTrackerOutput::createFalseOutput() {
    return std::make_shared<VROARImageTrackerOutput>(false);
}

VROARImageTrackerOutput::VROARImageTrackerOutput(bool found) :
    found(found) {
    
}

VROARImageTrackerOutput::VROARImageTrackerOutput(bool found, std::vector<cv::Point2f> corners,
                                             cv::Mat translation, cv::Mat rotation) :
    found(found), corners(corners), translation(translation), rotation(rotation) {
    
}

VROARImageTrackerOutput::~VROARImageTrackerOutput() {

}
