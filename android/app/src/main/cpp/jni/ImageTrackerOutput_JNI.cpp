//
//  ImageTrackerOutput_JNI.h
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

VRO_METHOD(VRO_FLOAT_ARRAY, nativeOutputCorners)(VRO_ARGS
                                                 jlong nativeRef) {
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
                                                  jlong nativeRef) {

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
                                                  jlong nativeRef) {

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