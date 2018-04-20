/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

#include <memory>
#include <VROPlatformUtil.h>
#include "PortalDelegate_JNI.h"

PortalDelegate::PortalDelegate(jobject javaPortalSceneObject){
    _javaObject = reinterpret_cast<jclass>(VROPlatformGetJNIEnv()->NewWeakGlobalRef(javaPortalSceneObject));
}

PortalDelegate::~PortalDelegate() {
    VROPlatformGetJNIEnv()->DeleteWeakGlobalRef(_javaObject);
}

void PortalDelegate::onPortalEnter() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "onPortalEnter", "()V");
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
    });
}



void PortalDelegate::onPortalExit() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "onPortalExit", "()V");
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
    });
}

