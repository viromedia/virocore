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
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "playerDidFinishPlaying", "()V");
        env->DeleteLocalRef(localObj);
    });
}

void VideoDelegate::videoDidFail(std::string error) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, error] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        jstring jerror = env->NewStringUTF(error.c_str());
        VROPlatformCallJavaFunction(localObj, "onVideoFailed", "(Ljava/lang/String;)V", jerror);
        env->DeleteLocalRef(localObj);
        env->DeleteLocalRef(jerror);
    });
}

void VideoDelegate::onReady() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(weakObj, "onReady", "()V");
        env->DeleteLocalRef(localObj);
    });
}

void VideoDelegate::onVideoUpdatedTime(int currentTimeInSeconds, int totalTimeInSeconds){
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, currentTimeInSeconds, totalTimeInSeconds] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(weakObj, "onVideoUpdatedTime", "(II)V",
                                    currentTimeInSeconds, totalTimeInSeconds);
        env->DeleteLocalRef(localObj);
    });
}