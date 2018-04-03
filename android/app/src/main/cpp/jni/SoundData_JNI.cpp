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
      Java_com_viro_core_SoundData_##method_name

extern "C" {

    JNI_METHOD(jlong, nativeCreateSoundData)(JNIEnv *env,
                                             jobject object,
                                             jstring filepath) {
        std::string path = VROPlatformGetString(filepath, env);

        // Set the platform env because the renderer could've not been initialized yet (and set
        // the env)
        VROPlatformSetEnv(env);
        std::shared_ptr<VROSoundDataGVR> data = VROSoundDataGVR::create(path, VROResourceType::URL);
        return SoundData::jptr(data);
    }

    JNI_METHOD(jlong, nativeSetSoundDataDelegate)(JNIEnv *env,
                                             jobject object,
                                             jlong nativeRef) {
        std::shared_ptr<VROSoundDataGVR> data = SoundData::native(nativeRef);
        std::shared_ptr<VROSoundDataDelegate_JNI> delegateRef
                = std::make_shared<VROSoundDataDelegate_JNI>(object, env);
        data->setDelegate(delegateRef);
        return SoundDataDelegate::jptr(delegateRef);
    }

    JNI_METHOD(void, nativeDestroySoundData)(JNIEnv *env,
                                             jobject obj,
                                             jlong nativeRef) {
        delete reinterpret_cast<PersistentRef<VROSoundDataGVR> *>(nativeRef);
    }

    JNI_METHOD(void, nativeDestroySoundDataDelegate)(JNIEnv *env,
                                             jobject obj,
                                             jlong nativeRef) {
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

        jstring jerror = env->NewStringUTF(error.c_str());
        VROPlatformCallJavaFunction(localObj, "dataError", "(Ljava/lang/String;)V", jerror);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
        env->DeleteLocalRef(jerror);
    });
}