/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

#include <jni.h>
#include <memory>
#include <VROPortalFrame.h>
#include "Portal_JNI.h"
#include "PersistentRef.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Portal_##method_name


extern "C" {

JNI_METHOD(jlong, nativeCreatePortal)(JNIEnv *env,
        jclass clazz) {
    std::shared_ptr<VROPortalFrame> portal = std::make_shared<VROPortalFrame>();
    return Portal::jptr(portal);
}

JNI_METHOD(void, nativeDestroyPortal)(JNIEnv *env,
                                                jclass clazz,
                                                jlong native_ref) {
    delete reinterpret_cast<PersistentRef<VROPortalFrame> *>(native_ref);
}

}