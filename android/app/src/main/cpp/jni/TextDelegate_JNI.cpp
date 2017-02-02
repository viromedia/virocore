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
    _javaObject = reinterpret_cast<jclass>(env->NewGlobalRef(textJavaObject));
}

TextDelegate::~TextDelegate() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->DeleteGlobalRef(_javaObject);
}

void TextDelegate::textCreated(long native_text_ref) {
    VROPlatformCallJavaFunction(_javaObject, "textDidFinishCreation", "(J)V", native_text_ref);
}
