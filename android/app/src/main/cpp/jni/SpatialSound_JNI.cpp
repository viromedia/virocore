//
//  SpatialSound_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#include <jni.h>
#include <memory>
#include <VROSound.h>
#include <VROSoundGVR.h>
#include <VROSoundDataGVR.h>
#include "PersistentRef.h"
#include "RenderContext_JNI.h"
#include "SoundDelegate_JNI.h"
#include "Node_JNI.h"
#include "SoundData_JNI.h"


#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_SpatialSoundJni_##method_name

namespace SpatialSound {
    inline jlong jptr(std::shared_ptr<VROSoundGVR> ptr) {
        PersistentRef<VROSoundGVR> *persistentRef = new PersistentRef<VROSoundGVR>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROSoundGVR> native(jlong ptr) {
        PersistentRef<VROSoundGVR> *persistentRef = reinterpret_cast<PersistentRef<VROSoundGVR> *>(ptr);
        return persistentRef->get();
    }
}

extern "C" {
JNI_METHOD(jlong, nativeCreateSpatialSound)(JNIEnv *env,
                                            jobject object,
                                            jstring filename,
                                            jboolean local,
                                            jlong renderContextRef) {
    std::shared_ptr<RenderContext> renderContext = RenderContext::native(renderContextRef);

    const char *cStrFile = env->GetStringUTFChars(filename, NULL);
    std::string file(cStrFile);
    env->ReleaseStringUTFChars(filename, cStrFile);

    std::shared_ptr<VROSound> soundEffect = renderContext->getDriver()->newSound(file, VROSoundType::Spatial, local);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(soundEffect);
    soundGvr->setDelegate(std::make_shared<SoundDelegate>(object));
    return SpatialSound::jptr(soundGvr);
}

JNI_METHOD(jlong, nativeCreateSpatialSoundWithData)(JNIEnv *env,
                                                    jobject object,
                                                    jlong dataRef,
                                                    jlong renderContextRef) {
    std::shared_ptr<RenderContext> renderContext = RenderContext::native(renderContextRef);
    std::shared_ptr<VROSoundDataGVR> data = SoundData::native(dataRef);

    std::shared_ptr<VROSound> sound = renderContext->getDriver()->newSound(data, VROSoundType::Spatial);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(sound);
    std::shared_ptr<SoundDelegate> delegate = std::make_shared<SoundDelegate>(object);
    soundGvr->setDelegate(delegate);

    return SpatialSound::jptr(soundGvr);
}

JNI_METHOD(void, nativeAttachToNode)(JNIEnv *env,
                                     jobject obj,
                                     jlong nativeSoundRef,
                                     jlong nativeNodeRef) {
    std::shared_ptr<VROSound> sound = SpatialSound::native(nativeSoundRef);
    Node::native(nativeNodeRef)->addSound(sound);
}

JNI_METHOD(void, nativeDetachFromNode)(JNIEnv *env,
                                       jobject obj,
                                       jlong nativeSoundRef,
                                       jlong nativeNodeRef) {
    std::shared_ptr<VROSound> sound = SpatialSound::native(nativeSoundRef);
    Node::native(nativeNodeRef)->removeSound(sound);
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