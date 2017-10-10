//
//  OpenCV_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//


/*
 * NOTE: THIS JNI CLASS IS CURRENTLY MEANT TO SIMPLY BE USED TO TEST THE CV FEATURES!
 */

#include <jni.h>
#include "opencv2/core/core.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <android/bitmap.h>
#include "VROLog.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_OpenCV_##method_name


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

JNI_METHOD(void, nativeRunEdgeDetection)(JNIEnv *env, jobject obj,
                                         jstring jinputFile,
                                         jstring joutputFile) {
    // Get the strings
    const char *cStrInput = env->GetStringUTFChars(jinputFile, NULL);
    std::string inputFileName(cStrInput);
    const char *cStrOutput = env->GetStringUTFChars(joutputFile, NULL);
    std::string outputFileName(cStrOutput);

    cv::Mat input = cv::imread(inputFileName, cv::IMREAD_GRAYSCALE);

    cv::Mat output = cv::Mat();
    cv::Canny(input, output, 70, 100);

    cv::imwrite(outputFileName, output);


    env->ReleaseStringUTFChars(jinputFile, cStrInput);
    env->ReleaseStringUTFChars(joutputFile, cStrOutput);
}

JNI_METHOD(void, nativeReadWriteBitmap)(JNIEnv *env, jobject obj,
                                        jstring jinstring, jstring joutstring) {
    const char *cStrInput = env->GetStringUTFChars(jinstring, NULL);
    std::string inputFilePath(cStrInput);
    const char *cStrOutput = env->GetStringUTFChars(joutstring, NULL);
    std::string outputFilePath(cStrOutput);

    cv::Mat input = cv::imread(inputFilePath, cv::IMREAD_GRAYSCALE);

    cv::imwrite(outputFilePath, input);

    env->ReleaseStringUTFChars(jinstring, cStrInput);
    env->ReleaseStringUTFChars(joutstring, cStrOutput);
}

}  // extern "C"
