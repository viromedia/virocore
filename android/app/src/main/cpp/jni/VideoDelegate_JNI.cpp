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
    _w_javaObject = VROPlatformGetJNIEnv()->NewWeakGlobalRef(javaVideoObject);
}

VideoDelegate::~VideoDelegate() {
    VROPlatformGetJNIEnv()->DeleteWeakGlobalRef(_w_javaObject);
}

void VideoDelegate::videoWillBuffer() {
    jweak weakObj = _w_javaObject;

    VROPlatformDispatchAsyncApplication([weakObj] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "playerWillBuffer", "()V");
        env->DeleteLocalRef(localObj);
    });
}

void VideoDelegate::videoDidBuffer() {
    jweak weakObj = _w_javaObject;

    VROPlatformDispatchAsyncApplication([weakObj] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "playerDidBuffer", "()V");
        env->DeleteLocalRef(localObj);
    });
}

void VideoDelegate::videoDidFinish() {
    jweak weakObj = _w_javaObject;

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
    jweak weakObj = _w_javaObject;

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
    jweak weakObj = _w_javaObject;

    VROPlatformDispatchAsyncApplication([weakObj] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "onReady", "()V");
        env->DeleteLocalRef(localObj);
    });
}

void VideoDelegate::onVideoUpdatedTime(float currentTimeInSeconds, float totalTimeInSeconds){
    jweak weakObj = _w_javaObject;

    VROPlatformDispatchAsyncApplication([weakObj, currentTimeInSeconds, totalTimeInSeconds] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "onVideoUpdatedTime", "(FF)V",
                                    currentTimeInSeconds, totalTimeInSeconds);
        env->DeleteLocalRef(localObj);
    });
}