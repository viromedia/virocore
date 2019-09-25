//
//  ImageTracker_JNI.cpp
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

#include <jni.h>
#include <memory>
#include "PersistentRef.h"
#include "ImageTrackerOutput_JNI.h"
#include <android/bitmap.h>

#if ENABLE_OPENCV
#include "VROARImageTracker.h"
#include <opencv2/imgproc.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "ARImageTarget_JNI.h"
#include "VROARImageTargetAndroid.h"

#endif /* ENABLE_OPENCV */

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_internal_ImageTracker_##method_name
#endif

#if ENABLE_OPENCV
namespace ImageTracker {
        inline jlong jptr(std::shared_ptr<VROARImageTracker> tracker) {
            PersistentRef<VROARImageTracker> *nativeTracker = new PersistentRef<VROARImageTracker>(tracker);
            return reinterpret_cast<intptr_t>(nativeTracker);
        }

        inline std::shared_ptr<VROARImageTracker> native(jlong ptr) {
            PersistentRef<VROARImageTracker> *persistentTracker = reinterpret_cast<PersistentRef<VROARImageTracker> *>(ptr);
            return persistentTracker->get();
        }
}
#endif /* ENABLE_OPENCV */

extern "C" {

#if ENABLE_OPENCV
// TODO: move this method somewhere else (VROPlatformUtils maybe?)
// This logic comes from OpenCV's java library: https://github.com/opencv/opencv/blob/master/modules/java/generator/src/cpp/utils.cpp
cv::Mat parseBitmapImage(JNIEnv *env, jobject bitmap) {
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
#endif /* ENABLE_OPENCV */

VRO_METHOD(jlong, nativeCreateImageTracker)(VRO_ARGS
                                            jlong imageTargetRef) {
#if ENABLE_OPENCV
    std::shared_ptr<VROARImageTargetAndroid> arImageTarget = std::dynamic_pointer_cast<VROARImageTargetAndroid>(VRO_REF_GET(VROARImageTarget, imageTargetRef));
    std::shared_ptr<VROImage> image = arImageTarget->getImage();

    size_t length;
    cv::Mat temp(image->getHeight(), image->getWidth(), CV_8UC4, image->getData(&length));
    arImageTarget->setTargetMat(temp);

    std::shared_ptr<VROARImageTracker> tracker = VROARImageTracker::createARImageTracker(arImageTarget);
    return ImageTracker::jptr(tracker);
#else
    return 0;
#endif /* ENABLE_OPENCV */
}

VRO_METHOD(jlong, nativeFindTarget)(VRO_ARGS
                                    jlong nativeRef,
                                    jobject bitmapImage) {
#if ENABLE_OPENCV
    cv::Mat imageMat = parseBitmapImage(env, bitmapImage);

    std::vector<VROARImageTrackerOutput> outputs = ImageTracker::native(nativeRef)->findTarget(imageMat, NULL);
    VROARImageTrackerOutput output;
    if (outputs.size() > 0) {
        output = outputs[0]; // grab the first one because there should only be one (only 1 target)
    } else {
        output = VROARImageTracker::createFalseOutput();
    }

    return ImageTrackerOutput::jptr(std::make_shared<VROARImageTrackerOutput>(output));
#else
    return 0;
#endif /* ENABLE_OPENCV */
}

}