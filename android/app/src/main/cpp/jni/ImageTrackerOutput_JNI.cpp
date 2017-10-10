//
//  ImageTrackerOutput_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "ImageTrackerOutput_JNI.h"
#include "opencv2/core/core.hpp"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_ImageTrackerOutput_##method_name

extern "C" {

JNI_METHOD(jboolean, nativeOutputFound)(JNIEnv *env,
                                        jobject obj,
                                        jlong nativeRef) {
    return ImageTrackerOutput::native(nativeRef)->found;
}

JNI_METHOD(jfloatArray, nativeOutputCorners)(JNIEnv *env,
                                             jobject obj,
                                             jlong nativeRef) {

    std::shared_ptr<VROImageTrackerOutput> output = ImageTrackerOutput::native(nativeRef);

    int returnLength = output->corners.size() * 2;
    jfloatArray returnCorners = env->NewFloatArray(returnLength);
    jfloat tempArr[returnLength];

    if (output->found) {
        for (int i = 0; i < output->corners.size(); i++) {
            tempArr[i * 2] = output->corners[i].x;
            tempArr[i * 2 + 1] = output->corners[i].y;
        }
    }
    env->SetFloatArrayRegion(returnCorners, 0, returnLength, tempArr);

    return returnCorners;
}

};