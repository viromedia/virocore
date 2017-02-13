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
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "nodeDidFinishCreation", "()V");
        env->DeleteLocalRef(localObj);
    });
}