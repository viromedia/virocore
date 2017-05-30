//
//  VROOpenCV.cpp
//  ViroRenderer
//
//  Created by Andy Chu on 5/25/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROOpenCV.h"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"

VROOpenCV::VROOpenCV() {
  
}

void VROOpenCV::runEdgeDetection(const char* inputFile, const char* outputFile) {
  
    cv::Mat input = cv::imread(inputFile, cv::IMREAD_GRAYSCALE);

    cv::Mat output = cv::Mat();
    pinfo("VROOpenCV mat sizes %d, %d", input.rows, input.cols);
    cv::Canny(input, output, 70, 100);
  
    cv::imwrite(outputFile, output);
}

void VROOpenCV::runEdgeDetection(cv::Mat inputFile, cv::Mat outputFile) {
    cv::Canny(inputFile, outputFile, 70, 100);
}
