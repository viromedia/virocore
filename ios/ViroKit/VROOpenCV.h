//
//  VROOpenCV.h
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

#ifndef VROOpenCV_h
#define VROOpenCV_h

#include <stdio.h>
#include <string>
#include <ViroKit/ViroKit.h>
#include "opencv2/core/core.hpp"


class VROOpenCV {
  
public:
    VROOpenCV();
    virtual ~VROOpenCV() = 0;

    static void runEdgeDetection(const char* inputFile, const char* outputFile);
  
    static void runEdgeDetection(cv::Mat inputFile, cv::Mat outputFile);
};
#endif /* VROOpenCV_h */

#endif /* ENABLE_OPENCV */
