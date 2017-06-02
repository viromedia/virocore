//
//  VROImageTrackerOutput.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROImageTrackerOutput.h"

std::shared_ptr<VROImageTrackerOutput> VROImageTrackerOutput::createFalseOutput() {
    return std::make_shared<VROImageTrackerOutput>(false);
}

VROImageTrackerOutput::VROImageTrackerOutput(bool found) :
    found(found) {
    
}

VROImageTrackerOutput::VROImageTrackerOutput(bool found, std::vector<cv::Point2f> corners,
                                             cv::Mat translation, cv::Mat rotation) :
    found(found), corners(corners), translation(translation), rotation(rotation) {
    
}

VROImageTrackerOutput::~VROImageTrackerOutput() {

}
