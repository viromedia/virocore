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
#include "VRONode.h"
#include "VideoDelegate_JNI.h"
#include "VROLog.h"

VideoDelegate::VideoDelegate(jobject javaVideoObject, JNIEnv *env){
    _javaObject = reinterpret_cast<jclass>(env->NewGlobalRef(javaVideoObject));
    _env = env;
}

VideoDelegate::~VideoDelegate() {
    _env->DeleteGlobalRef(_javaObject);
}

void VideoDelegate::videoDidFinish() {
    _env->ExceptionClear();
    jclass viroClass = _env->FindClass("com/viro/renderer/jni/VideoTextureJni");
    if (viroClass == nullptr) {
        perr("Unable to find VideoSurfaceJni class for playerDidFinishPlaying() callback.");
        return;
    }

    jmethodID method = _env->GetMethodID(viroClass, "playerDidFinishPlaying", "()V");
    if (method == nullptr) {
        perr("Unable to find method playerDidFinishPlaying() callback.");
        return;
    }

    _env->CallVoidMethod(_javaObject, method);
    if (_env->ExceptionOccurred()) {
        perr("Exception occured when calling nativeOnVideoFinished.");
        _env->ExceptionClear();
    }
    _env->DeleteLocalRef(viroClass);
}