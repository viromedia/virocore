/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

#include <memory>
#include <VROPlatformUtil.h>
#include "PortalDelegate_JNI.h"


PortalDelegate::PortalDelegate(jobject javaPortalSceneObject){
    _w_javaObject = VROPlatformGetJNIEnv()->NewWeakGlobalRef(javaPortalSceneObject);
}

PortalDelegate::~PortalDelegate() {
    VROPlatformGetJNIEnv()->DeleteWeakGlobalRef(_w_javaObject);
}

void PortalDelegate::onPortalEnter() {
    jweak weakObj = _w_javaObject;

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
    jweak weakObj = _w_javaObject;

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

