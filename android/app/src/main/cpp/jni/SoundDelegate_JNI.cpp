//
//  SoundDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <jni.h>
#include <VROPlatformUtil.h>
#include <VROLog.h>
#include "SoundDelegate_JNI.h"

SoundDelegate::SoundDelegate(jobject soundObjectJava) {
    _javaObject = reinterpret_cast<jclass>(VROPlatformGetJNIEnv()->NewWeakGlobalRef(soundObjectJava));
}

SoundDelegate::~SoundDelegate() {
    // TODO: fix the below
    VROPlatformGetJNIEnv()->DeleteWeakGlobalRef(_javaObject);
}

void SoundDelegate::soundIsReady() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->ExceptionClear();
    jclass javaClass = env->FindClass("com/viro/core/internal/NativeSoundDelegate");

    if (javaClass == nullptr) {
        perr("Unable to find NativeSoundDelegate class for soundIsReady() callback");
        return;
    }

    jmethodID method = env->GetMethodID(javaClass, "soundIsReady", "()V");
    if (method == nullptr) {
        perr("Unable to find method soundIsReady()");
        return;
    }

    env->CallVoidMethod(_javaObject, method);
    if (env->ExceptionOccurred()) {
        perr("Exception occurred while invoking soundIsReady()");
        env->ExceptionDescribe();
        std::string errorString = "A java exception has been thrown when calling soundIsReady";
        throw std::runtime_error(errorString.c_str());
    }
    env->DeleteLocalRef(javaClass);
}

void SoundDelegate::soundDidFinish() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->ExceptionClear();
    jclass javaClass = env->FindClass("com/viro/core/internal/NativeSoundDelegate");

    if (javaClass == nullptr) {
        perr("Unable to find NativeSoundDelegate class for soundDidFinish() callback");
        return;
    }

    jmethodID method = env->GetMethodID(javaClass, "soundDidFinish", "()V");
    if (method == nullptr) {
        perr("Unable to find method soundIsFinished()");
        return;
    }

    env->CallVoidMethod(_javaObject, method);
    if (env->ExceptionOccurred()) {
        perr("Exception occurred while invoking soundDidFinish()");
        env->ExceptionDescribe();
        std::string errorString = "A java exception has been thrown when calling soundDidFinish";
        throw std::runtime_error(errorString.c_str());
    }
    env->DeleteLocalRef(javaClass);
}

void SoundDelegate::soundDidFail(std::string error) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->ExceptionClear();
    jclass javaClass = env->FindClass("com/viro/core/internal/NativeSoundDelegate");

    if (javaClass == nullptr) {
        perr("Unable to find NativeSoundDelegate class for soundDidFail(String) callback");
        return;
    }

    jmethodID method = env->GetMethodID(javaClass, "soundDidFail", "(Ljava/lang/String;)V");
    if (method == nullptr) {
        perr("Unable to find method soundIsFail(String)");
        return;
    }

    jstring jerror = env->NewStringUTF(error.c_str());
    env->CallVoidMethod(_javaObject, method, jerror);
    if (env->ExceptionOccurred()) {
        perr("Exception occurred while invoking soundDidFail()");
        env->ExceptionDescribe();
        std::string errorString = "A java exception has been thrown when calling soundDidFail";
        throw std::runtime_error(errorString.c_str());
    }

    env->DeleteLocalRef(jerror);
    env->DeleteLocalRef(javaClass);
}
