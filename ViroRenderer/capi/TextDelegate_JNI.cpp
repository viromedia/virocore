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

TextDelegate::TextDelegate(VRO_OBJECT textJavaObject, JNIEnv *env) {
    _javaObject = reinterpret_cast<jclass>(VRO_NEW_WEAK_GLOBAL_REF(textJavaObject));
}

TextDelegate::~TextDelegate() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
}

void TextDelegate::textCreated(VRO_REF nativeTextRef) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, nativeTextRef] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "textDidFinishCreation", "(J)V", nativeTextRef);
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}
