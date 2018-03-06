//
//  ImageTracker_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include <opencv2/imgproc.hpp>
#include "PersistentRef.h"
#include "ImageTrackerOutput_JNI.h"
#include "VROARImageTracker.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include <android/bitmap.h>


#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_internal_ImageTracker_##method_name

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

JNI_METHOD(jlong, nativeCreateImageTracker)(JNIEnv *env,
                                            jclass clazz,
                                            jobject bitmapImage) {
#if ENABLE_OPENCV
    cv::Mat image = parseBitmapImage(env, bitmapImage);

    std::shared_ptr<VROARImageTracker> tracker = VROARImageTracker::createARImageTracker(image);

    return ImageTracker::jptr(tracker);
#else
    return 0;
#endif /* ENABLE_OPENCV */
}

JNI_METHOD(jlong, nativeFindTarget)(JNIEnv *env,
                                    jclass clazz,
                                    jlong nativeRef,
                                    jobject bitmapImage) {
#if ENABLE_OPENCV
    cv::Mat imageMat = parseBitmapImage(env, bitmapImage);

    std::shared_ptr<VROARImageTrackerOutput> output = ImageTracker::native(nativeRef)->findTarget(imageMat);

    return ImageTrackerOutput::jptr(output);
#else
    return 0;
#endif /* ENABLE_OPENCV */
}

}