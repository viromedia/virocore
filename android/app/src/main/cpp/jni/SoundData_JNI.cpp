//
//  SoundData_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#include <jni.h>
#include <VROPlatformUtil.h>
#include "SoundData_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_SoundData_##method_name
#endif

extern "C" {

    VRO_METHOD(VRO_REF, nativeCreateSoundData)(VRO_ARGS
                                               VRO_STRING filepath) {
        std::string path = VROPlatformGetString(filepath, env);

        // Set the platform env because the renderer could've not been initialized yet (and set
        // the env)
        VROPlatformSetEnv(env);
        std::shared_ptr<VROSoundDataGVR> data = VROSoundDataGVR::create(path, VROResourceType::URL);
        return SoundData::jptr(data);
    }

    VRO_METHOD(VRO_REF, nativeSetSoundDataDelegate)(VRO_ARGS
                                                    VRO_REF nativeRef) {
        std::shared_ptr<VROSoundDataGVR> data = SoundData::native(nativeRef);
        std::shared_ptr<VROSoundDataDelegate_JNI> delegateRef
                = std::make_shared<VROSoundDataDelegate_JNI>(obj, env);
        data->setDelegate(delegateRef);
        return SoundDataDelegate::jptr(delegateRef);
    }

    VRO_METHOD(void, nativeDestroySoundData)(VRO_ARGS
                                             VRO_REF nativeRef) {
        delete reinterpret_cast<PersistentRef<VROSoundDataGVR> *>(nativeRef);
    }

    VRO_METHOD(void, nativeDestroySoundDataDelegate)(VRO_ARGS
                                                    VRO_REF nativeRef) {
        delete reinterpret_cast<PersistentRef<VROSoundDataDelegate_JNI> *>(nativeRef);
    }
} // extern "C"

VROSoundDataDelegate_JNI::VROSoundDataDelegate_JNI(jobject nodeJavaObject, JNIEnv *env) {
    _javaObject = reinterpret_cast<jclass>(env->NewWeakGlobalRef(nodeJavaObject));
}

VROSoundDataDelegate_JNI::~VROSoundDataDelegate_JNI() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->DeleteWeakGlobalRef(_javaObject);
}

void VROSoundDataDelegate_JNI::dataIsReady(){
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(weakObj);
            return;
        }

        VROPlatformCallJavaFunction(localObj, "dataIsReady", "()V");
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
    });
}

void VROSoundDataDelegate_JNI::dataError(std::string error){
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, error] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(weakObj);
            return;
        }

        VRO_STRING jerror = VRO_NEW_STRING(error.c_str());
        VROPlatformCallJavaFunction(localObj, "dataError", "(Ljava/lang/String;)V", jerror);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
        env->DeleteLocalRef(jerror);
    });
}