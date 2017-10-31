//
//  SpatialSound_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include "SpatialSound_JNI.h"
#include <VROSound.h>
#include <VROSoundDataGVR.h>
#include "ViroContext_JNI.h"
#include "SoundDelegate_JNI.h"
#include "SoundData_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_SpatialSound_##method_name

extern "C" {
JNI_METHOD(jlong, nativeCreateSpatialSound)(JNIEnv *env,
                                            jobject object,
                                            jstring filename,
                                            jboolean local,
                                            jlong context_j) {
    std::shared_ptr<ViroContext> context = ViroContext::native(context_j);

    const char *cStrFile = env->GetStringUTFChars(filename, NULL);
    std::string file(cStrFile);
    env->ReleaseStringUTFChars(filename, cStrFile);

    std::shared_ptr<VROSound> soundEffect = context->getDriver()->newSound(file, VROSoundType::Spatial, local);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(soundEffect);
    soundGvr->setDelegate(std::make_shared<SoundDelegate>(object));
    return SpatialSound::jptr(soundGvr);
}

JNI_METHOD(jlong, nativeCreateSpatialSoundWithData)(JNIEnv *env,
                                                    jobject object,
                                                    jlong dataRef,
                                                    jlong context_j) {
    std::shared_ptr<ViroContext> context = ViroContext::native(context_j);
    std::shared_ptr<VROSoundDataGVR> data = SoundData::native(dataRef);

    std::shared_ptr<VROSound> sound = context->getDriver()->newSound(data, VROSoundType::Spatial);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(sound);
    std::shared_ptr<SoundDelegate> delegate = std::make_shared<SoundDelegate>(object);
    soundGvr->setDelegate(delegate);

    return SpatialSound::jptr(soundGvr);
}

JNI_METHOD(void, nativePlaySpatialSound)(JNIEnv *env, jobject obj, jlong nativeRef) {
    SpatialSound::native(nativeRef)->play();
}

JNI_METHOD(void, nativePauseSpatialSound)(JNIEnv *env, jobject obj, jlong nativeRef) {
    SpatialSound::native(nativeRef)->pause();
}

JNI_METHOD(void, nativeSetVolume)(JNIEnv *env, jobject obj,
                                  jlong nativeRef,
                                  jfloat volume) {
    SpatialSound::native(nativeRef)->setVolume(volume);
}

JNI_METHOD(void, nativeSetMuted)(JNIEnv *env, jobject obj,
                                 jlong nativeRef,
                                 jboolean muted) {
    SpatialSound::native(nativeRef)->setMuted(muted);
}

JNI_METHOD(void, nativeSetLoop)(JNIEnv *env, jobject obj,
                                jlong nativeRef,
                                jboolean loop) {
    SpatialSound::native(nativeRef)->setLoop(loop);
}

JNI_METHOD(void, nativeSeekToTime)(JNIEnv *env, jobject obj,
                                   jlong nativeRef,
                                   jfloat seconds) {
    SpatialSound::native(nativeRef)->seekToTime(seconds);
}

JNI_METHOD(void, nativeSetPosition)(JNIEnv *env,
                                    jobject obj,
                                    jlong nativeRef,
                                    jfloat posX,
                                    jfloat posY,
                                    jfloat posZ) {
    SpatialSound::native(nativeRef)->setPosition({posX, posY, posZ});
}

JNI_METHOD(void, nativeSetDistanceRolloff)(JNIEnv *env,
                                           jobject obj,
                                           jlong nativeRef,
                                           jstring model,
                                           jfloat minDistance,
                                           jfloat maxDistance) {
    const char *cModelString = env->GetStringUTFChars(model, NULL);
    std::string modelString(cModelString);

    if (VROStringUtil::strcmpinsensitive(modelString, "none")) {
        SpatialSound::native(nativeRef)->setDistanceRolloffModel(VROSoundRolloffModel::None,
                                                                 minDistance, maxDistance);
    } else if (VROStringUtil::strcmpinsensitive(modelString, "linear")) {
        SpatialSound::native(nativeRef)->setDistanceRolloffModel(VROSoundRolloffModel::Linear,
                                                                 minDistance, maxDistance);
    } else if (VROStringUtil::strcmpinsensitive(modelString, "logarithmic")) {
        SpatialSound::native(nativeRef)->setDistanceRolloffModel(VROSoundRolloffModel::Logarithmic,
                                                                 minDistance, maxDistance);
    }
}

JNI_METHOD(void, nativeDestroySpatialSound)(JNIEnv *env, jobject obj, jlong nativeRef) {
    SpatialSound::native(nativeRef)->setDelegate(nullptr);
    delete reinterpret_cast<PersistentRef<VROSoundGVR> *>(nativeRef);
}

}  // extern "C"