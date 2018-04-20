//
//  ImageTrackerOutput_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "ImageTrackerOutput_JNI.h"
#include "opencv2/core/core.hpp"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_internal_ImageTrackerOutput_##method_name
#endif

extern "C" {

VRO_METHOD(jboolean, nativeOutputFound)(VRO_ARGS
                                        jlong nativeRef) {
#if ENABLE_OPENCV
    return ImageTrackerOutput::native(nativeRef)->found;
#else
    return false;
#endif /* ENABLE_OPENCV */
}

VRO_METHOD(jfloatArray, nativeOutputCorners)(VRO_ARGS
                                             jlong nativeRef) {
#if ENABLE_OPENCV

    std::shared_ptr<VROARImageTrackerOutput> output = ImageTrackerOutput::native(nativeRef);

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
#else
    return nullptr;
#endif /* ENABLE_OPENCV */

}

VRO_METHOD(jfloatArray, nativeOutputPosition)(VRO_ARGS
                                              jlong nativeRef) {

    jfloatArray returnPosition = env->NewFloatArray(3);
    jfloat tempArr[3];

#if ENABLE_OPENCV
    std::shared_ptr<VROARImageTrackerOutput> output = ImageTrackerOutput::native(nativeRef);

    // We have to negate the y and z rotation because OpenCV y and z axis are opposite from ours.
    tempArr[0] = output->translation.at<double>(0,0);
    tempArr[1] = - output->translation.at<double>(1,0);
    tempArr[2] = - output->translation.at<double>(2,0);

    env->SetFloatArrayRegion(returnPosition, 0, 3, tempArr);
#endif /* ENABLE_OPENCV */

    return returnPosition;
}

VRO_METHOD(jfloatArray, nativeOutputRotation)(VRO_ARGS
                                              jlong nativeRef) {

    jfloatArray returnRotation = env->NewFloatArray(3);
    jfloat tempArr[3];

#if ENABLE_OPENCV
    std::shared_ptr<VROARImageTrackerOutput> output = ImageTrackerOutput::native(nativeRef);

    // We have to negate the y and z rotation because OpenCV y and z axis are opposite from ours.
    tempArr[0] = output->rotation.at<double>(0,0);
    tempArr[1] = - output->rotation.at<double>(1,0);
    tempArr[2] = - output->rotation.at<double>(2,0);

    env->SetFloatArrayRegion(returnRotation, 0, 3, tempArr);
#endif /* ENABLE_OPENCV */

    return returnRotation;
}

};