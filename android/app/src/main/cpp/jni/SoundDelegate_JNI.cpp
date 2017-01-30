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
    _javaObject = reinterpret_cast<jclass>(VROPlatformGetJNIEnv()->NewGlobalRef(soundObjectJava));
}

SoundDelegate::~SoundDelegate() {
    // TODO: fix the below
    VROPlatformGetJNIEnv()->DeleteGlobalRef(_javaObject);
}

void SoundDelegate::soundIsReady() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->ExceptionClear();
    jclass javaClass = env->FindClass("com/viro/renderer/jni/NativeSoundDelegate");

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
        env->ExceptionClear();
    }
    env->DeleteLocalRef(javaClass);
}

void SoundDelegate::soundDidFinish() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->ExceptionClear();
    jclass javaClass = env->FindClass("com/viro/renderer/jni/NativeSoundDelegate");

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
        env->ExceptionClear();
    }
    env->DeleteLocalRef(javaClass);
}