//
//  VROOpenCV.cpp
//  ViroRenderer
//
//  Created by Andy Chu on 5/25/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#if ENABLE_OPENCV

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

#endif /* ENABLE_OPENCV */
