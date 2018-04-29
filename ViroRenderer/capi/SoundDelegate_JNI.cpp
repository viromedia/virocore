//
//  SoundDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include <VROLog.h>
#include "SoundDelegate_JNI.h"

SoundDelegate::SoundDelegate(VRO_OBJECT soundObjectJava) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    _javaObject = reinterpret_cast<jclass>(VRO_NEW_WEAK_GLOBAL_REF(soundObjectJava));
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
