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
    VROPlatformCallJavaFunction(_javaObject, "soundIsReady", "()V");
}

void SoundDelegate::soundDidFinish() {
    VROPlatformCallJavaFunction(_javaObject, "soundDidFinish", "()V");
}

void SoundDelegate::soundDidFail(std::string error) {
    VROPlatformCallJavaFunction(_javaObject, "soundDidFail", "(Ljava/lang/String;)V");
}
