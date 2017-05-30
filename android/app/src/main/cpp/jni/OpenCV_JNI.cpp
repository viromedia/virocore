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

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_OpenCVJni_##method_name


extern "C" {


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

}  // extern "C"
