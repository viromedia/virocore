//
//  TextDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include "TextDelegate_JNI.h"

#include <VROPlatformUtil.h>

TextDelegate::TextDelegate(jobject textJavaObject, JNIEnv *env) {
    _javaObject = reinterpret_cast<jclass>(env->NewWeakGlobalRef(textJavaObject));
}

TextDelegate::~TextDelegate() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->DeleteWeakGlobalRef(_javaObject);
}

void TextDelegate::textCreated(jlong nativeTextRef) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, nativeTextRef] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "textDidFinishCreation", "(J)V", nativeTextRef);
        env->DeleteLocalRef(localObj);
    });
}
