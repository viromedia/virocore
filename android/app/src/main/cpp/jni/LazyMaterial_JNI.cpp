//
//  LazyMaterial_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//


#include "LazyMaterial_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_LazyMaterial_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateLazyMaterial)(JNIEnv *env, jobject obj) {
    std::shared_ptr<VROLazyMaterialJNI> materialPtr = std::make_shared<VROLazyMaterialJNI>(obj);
    return LazyMaterial::jptr(materialPtr);
}

JNI_METHOD(void, nativeDestroyLazyMaterial)(JNIEnv *env,
                                            jobject obj,
                                            jlong nativeRef) {
    delete reinterpret_cast<PersistentRef<VROLazyMaterialJNI> *>(nativeRef);
}

}  // extern "C"