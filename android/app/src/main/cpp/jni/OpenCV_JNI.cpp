//
//  OpenCV_JNI.cpp
//  ViroRenderer
//
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

/*
 * NOTE: THIS JNI CLASS IS CURRENTLY MEANT TO SIMPLY BE USED TO TEST THE CV FEATURES!
 */

#include <jni.h>
#include "opencv2/core/core.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <android/bitmap.h>
#include <VROPlatformUtil.h>
#include "VROLog.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_OpenCV_##method_name
#endif

extern "C" {

cv::Mat openCVParseBitmapImage(JNIEnv *env, jobject bitmap) {
    AndroidBitmapInfo info;
    void* pixels = 0;
    cv::Mat toReturn;

    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    toReturn.create(info.height, info.width, CV_8UC4);

    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat tmp(info.height, info.width, CV_8UC4, pixels);
        tmp.copyTo(toReturn);
    } else { // info.format == ANDROID_BITMAP_FORMAT_RGB_565
        cv::Mat tmp(info.height, info.width, CV_8UC2, pixels);
        cvtColor(tmp, toReturn, cv::COLOR_BGR5652RGBA);
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    return toReturn;
}

VRO_METHOD(void, nativeRunEdgeDetection)(VRO_ARGS
                                         VRO_STRING jinputFile,
                                         VRO_STRING joutputFile) {
    // Get the strings
    std::string inputFileName = VRO_STRING_STL(jinputFile);
    std::string outputFileName = VRO_STRING_STL(joutputFile);

    cv::Mat input = cv::imread(inputFileName, cv::IMREAD_GRAYSCALE);

    cv::Mat output = cv::Mat();
    cv::Canny(input, output, 70, 100);

    cv::imwrite(outputFileName, output);
}

VRO_METHOD(void, nativeReadWriteBitmap)(VRO_ARGS
                                        VRO_STRING jinstring, VRO_STRING joutstring) {
    std::string inputFilePath = VRO_STRING_STL(jinstring);
    std::string outputFilePath = VRO_STRING_STL(joutstring);

    cv::Mat input = cv::imread(inputFilePath, cv::IMREAD_GRAYSCALE);

    cv::imwrite(outputFilePath, input);
}

}  // extern "C"
