//
//  VRORendererARCore_JNI.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 2/24/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include <jni.h>
#include "ARCore_Native.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_RendererARCore_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateARCoreSession)(JNIEnv *env, jobject object, jobject context) {
    arcore::Session *session = new arcore::SessionNative(context, env);
    return reinterpret_cast<jlong>(session);
}

}



