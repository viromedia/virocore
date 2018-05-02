//
//  SoundDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include <VROLog.h>
#include "SoundDelegate_JNI.h"

SoundDelegate::SoundDelegate(VRO_OBJECT soundObjectJava) :
    _javaObject(VRO_OBJECT_NULL) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    _javaObject = VRO_NEW_WEAK_GLOBAL_REF(soundObjectJava);
}

SoundDelegate::~SoundDelegate() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
}

void SoundDelegate::soundIsReady() {
    VROPlatformCallHostFunction(_javaObject, "soundIsReady", "()V");
}

void SoundDelegate::soundDidFinish() {
    VROPlatformCallHostFunction(_javaObject, "soundDidFinish", "()V");
}

void SoundDelegate::soundDidFail(std::string error) {
    VROPlatformCallHostFunction(_javaObject, "soundDidFail", "(Ljava/lang/String;)V");
}
