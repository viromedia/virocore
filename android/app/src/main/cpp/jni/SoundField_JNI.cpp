//
//  SoundField_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#include <jni.h>
#include <memory>
#include <VROSound.h>
#include <VROSoundGVR.h>
#include "PersistentRef.h"
#include "RenderContext_JNI.h"
#include "SoundDelegate_JNI.h"
#include "SoundData_JNI.h"


#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_SoundField_##method_name

namespace SoundField {
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

JNI_METHOD(jlong, nativeCreateSoundField)(JNIEnv *env,
                                          jobject object,
                                          jstring filename,
                                          jboolean local,
                                          jlong renderContextRef) {
    std::shared_ptr<RenderContext> renderContext = RenderContext::native(renderContextRef);

    const char *cStrFile = env->GetStringUTFChars(filename, NULL);
    std::string file(cStrFile);
    env->ReleaseStringUTFChars(filename, cStrFile);

    std::shared_ptr<VROSound> soundEffect = renderContext->getDriver()->newSound(file, VROSoundType::SoundField, local);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(soundEffect);
    soundGvr->setDelegate(std::make_shared<SoundDelegate>(object));

    return SoundField::jptr(soundGvr);
}

JNI_METHOD(jlong, nativeCreateSoundFieldWithData)(JNIEnv *env,
                                                  jobject object,
                                                  jlong dataRef,
                                                  jlong renderContextRef) {
    std::shared_ptr<RenderContext> renderContext = RenderContext::native(renderContextRef);
    std::shared_ptr<VROSoundDataGVR> data = SoundData::native(dataRef);

    std::shared_ptr<VROSound> sound = renderContext->getDriver()->newSound(data, VROSoundType::SoundField);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(sound);
    soundGvr->setDelegate(std::make_shared<SoundDelegate>(object));

    return SoundField::jptr(soundGvr);
}

JNI_METHOD(void, nativePlaySoundField)(JNIEnv *env, jobject obj, jlong nativeRef) {
    SoundField::native(nativeRef)->play();
}

JNI_METHOD(void, nativePauseSoundField)(JNIEnv *env, jobject obj, jlong nativeRef) {
    SoundField::native(nativeRef)->pause();
}

JNI_METHOD(void, nativeSetVolume)(JNIEnv *env, jobject obj,
                                  jlong nativeRef,
                                  jfloat volume) {
    SoundField::native(nativeRef)->setVolume(volume);
}

JNI_METHOD(void, nativeSetMuted)(JNIEnv *env, jobject obj,
                                 jlong nativeRef,
                                 jboolean muted) {
    SoundField::native(nativeRef)->setMuted(muted);
}

JNI_METHOD(void, nativeSetLoop)(JNIEnv *env, jobject obj,
                                jlong nativeRef,
                                jboolean loop) {
    SoundField::native(nativeRef)->setLoop(loop);
}

JNI_METHOD(void, nativeSeekToTime)(JNIEnv *env, jobject obj,
                                   jlong nativeRef,
                                   jfloat seconds) {
    SoundField::native(nativeRef)->seekToTime(seconds);
}

JNI_METHOD(void, nativeSetRotation)(JNIEnv *env,
                                    jobject obj,
                                    jlong nativeRef,
                                    jfloat rotationDegreesX,
                                    jfloat rotationDegreesY,
                                    jfloat rotationDegreesZ) {
    SoundField::native(nativeRef)->setRotation({toRadians(rotationDegreesX),
                                                toRadians(rotationDegreesY),
                                                toRadians(rotationDegreesZ)});
}

JNI_METHOD(void, nativeDestroySoundField)(JNIEnv *env, jobject obj, jlong nativeRef) {
    delete reinterpret_cast<PersistentRef<VROSoundGVR> *>(nativeRef);
}

}  // extern "C"