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
                                        VRO_REF nativeRef) {
#if ENABLE_OPENCV
    return ImageTrackerOutput::native(nativeRef)->found;
#else
    return false;
#endif /* ENABLE_OPENCV */
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeOutputCorners)(VRO_ARGS
                                                 VRO_REF nativeRef) {
#if ENABLE_OPENCV

    std::shared_ptr<VROARImageTrackerOutput> output = ImageTrackerOutput::native(nativeRef);

    int returnLength = output->corners.size() * 2;
    VRO_FLOAT_ARRAY returnCorners = VRO_NEW_FLOAT_ARRAY(returnLength);
    VRO_FLOAT tempArr[returnLength];

    if (output->found) {
        for (int i = 0; i < output->corners.size(); i++) {
            tempArr[i * 2] = output->corners[i].x;
            tempArr[i * 2 + 1] = output->corners[i].y;
        }
    }
    VRO_FLOAT_ARRAY_SET(returnCorners, 0, returnLength, tempArr);

    return returnCorners;
#else
    return nullptr;
#endif /* ENABLE_OPENCV */

}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeOutputPosition)(VRO_ARGS
                                                  VRO_REF nativeRef) {

    VRO_FLOAT_ARRAY returnPosition = VRO_NEW_FLOAT_ARRAY(3);
    VRO_FLOAT tempArr[3];

#if ENABLE_OPENCV
    std::shared_ptr<VROARImageTrackerOutput> output = ImageTrackerOutput::native(nativeRef);

    // We have to negate the y and z rotation because OpenCV y and z axis are opposite from ours.
    tempArr[0] = output->translation.at<double>(0,0);
    tempArr[1] = - output->translation.at<double>(1,0);
    tempArr[2] = - output->translation.at<double>(2,0);

    VRO_FLOAT_ARRAY_SET(returnPosition, 0, 3, tempArr);
#endif /* ENABLE_OPENCV */

    return returnPosition;
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeOutputRotation)(VRO_ARGS
                                                  VRO_REF nativeRef) {

    VRO_FLOAT_ARRAY returnRotation = VRO_NEW_FLOAT_ARRAY(3);
    VRO_FLOAT tempArr[3];

#if ENABLE_OPENCV
    std::shared_ptr<VROARImageTrackerOutput> output = ImageTrackerOutput::native(nativeRef);

    // We have to negate the y and z rotation because OpenCV y and z axis are opposite from ours.
    tempArr[0] = output->rotation.at<double>(0,0);
    tempArr[1] = - output->rotation.at<double>(1,0);
    tempArr[2] = - output->rotation.at<double>(2,0);

    VRO_FLOAT_ARRAY_SET(returnRotation, 0, 3, tempArr);
#endif /* ENABLE_OPENCV */

    return returnRotation;
}

};