//
//  OBJLoaderDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//


#include <jni.h>
#include <memory>
#include "VROPlatformUtil.h"
#include "OBJLoaderDelegate_JNI.h"

OBJLoaderDelegate::OBJLoaderDelegate(jobject nodeJavaObject, JNIEnv *env) {
    _javaObject = reinterpret_cast<jclass>(env->NewGlobalRef(nodeJavaObject));
}

OBJLoaderDelegate::~OBJLoaderDelegate() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->DeleteGlobalRef(_javaObject);
}

void OBJLoaderDelegate::objLoaded() {
    VROPlatformCallJavaFunction(_javaObject, "nodeDidFinishCreation", "()V");
}