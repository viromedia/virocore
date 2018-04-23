//
//  VRORendererARCore_JNI.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 2/24/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "ARCore_Native.h"
#include "VRODefines.h"
#include VRO_C_INCLUDE

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_RendererARCore_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateARCoreSession)(JNIEnv *env, jobject object, jobject context) {
    arcore::Session *session = new arcore::SessionNative(context, env);
    return reinterpret_cast<VRO_REF>(session);
}

}



