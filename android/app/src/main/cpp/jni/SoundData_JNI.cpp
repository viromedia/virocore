//
//  SoundData_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#include <jni.h>
#include <VROPlatformUtil.h>
#include "SoundData_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_SoundData_##method_name

extern "C" {

    JNI_METHOD(jlong, nativeCreateSoundData)(JNIEnv *env,
                                             jclass clazz,
                                             jstring filepath,
                                             jboolean local) {
        const char *cStrPath = env->GetStringUTFChars(filepath, NULL);
        std::string path(cStrPath);
        env->ReleaseStringUTFChars(filepath, cStrPath);

        // Set the platform env because the renderer could've not been initialized yet (and set
        // the env)
        VROPlatformSetEnv(env);
        std::shared_ptr<VROSoundDataGVR> data = VROSoundDataGVR::create(path, local);
        return SoundData::jptr(data);
    }

    JNI_METHOD(void, nativeDestroySoundData)(JNIEnv *env,
                                             jobject obj,
                                             jlong nativeRef) {
        delete reinterpret_cast<PersistentRef<VROSoundDataGVR> *>(nativeRef);
    }

} // extern "C"

