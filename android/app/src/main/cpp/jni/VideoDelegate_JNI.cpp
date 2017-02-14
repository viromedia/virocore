//
//  VideoDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include <VROVideoSurface.h>
#include <VROVideoTextureAVP.h>
#include <VROPlatformUtil.h>
#include "VRONode.h"
#include "VideoDelegate_JNI.h"
#include "VROLog.h"

VideoDelegate::VideoDelegate(jobject javaVideoObject){
    _javaObject = reinterpret_cast<jclass>(VROPlatformGetJNIEnv()->NewGlobalRef(javaVideoObject));
}

VideoDelegate::~VideoDelegate() {
    VROPlatformGetJNIEnv()->DeleteGlobalRef(_javaObject);
}

void VideoDelegate::videoDidFinish() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->ExceptionClear();
    jclass viroClass = env->FindClass("com/viro/renderer/jni/VideoTextureJni");
    if (viroClass == nullptr) {
        perr("Unable to find VideoTextureJni class for playerDidFinishPlaying() callback.");
        return;
    }

    jmethodID method = env->GetMethodID(viroClass, "playerDidFinishPlaying", "()V");
    if (method == nullptr) {
        perr("Unable to find method playerDidFinishPlaying() callback.");
        return;
    }

    env->CallVoidMethod(_javaObject, method);
    if (env->ExceptionOccurred()) {
        perr("Exception occured when calling nativeOnVideoFinished.");
        env->ExceptionClear();
    }
    env->DeleteLocalRef(viroClass);
}