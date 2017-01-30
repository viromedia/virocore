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


#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_SoundFieldJni_##method_name

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
JNI_METHOD(jlong, nativeCreateSoundFieldFromFile)(JNIEnv *env,
                                             jobject object,
                                             jstring filename,
                                             jlong renderContextRef) {
    std::shared_ptr<RenderContext> renderContext = RenderContext::native(renderContextRef);

    const char *cStrFile = env->GetStringUTFChars(filename, NULL);
    std::string file(cStrFile);
    env->ReleaseStringUTFChars(filename, cStrFile);

    // TODO: VIRO-756 do something different for local files
    std::shared_ptr<VROSound> soundEffect = renderContext->getDriver()->newSound(file, VROSoundType::SoundField);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(soundEffect);
    soundGvr->setDelegate(std::make_shared<SoundDelegate>(object));

    return SoundField::jptr(soundGvr);
}

JNI_METHOD(jlong, nativeCreateSoundFieldFromUrl)(JNIEnv *env,
                                            jobject object,
                                            jstring filename,
                                            jlong renderContextRef) {
    std::shared_ptr<RenderContext> renderContext = RenderContext::native(renderContextRef);

    const char *cStrFile = env->GetStringUTFChars(filename, NULL);
    std::string file(cStrFile);
    env->ReleaseStringUTFChars(filename, cStrFile);

    std::shared_ptr<VROSound> soundEffect = renderContext->getDriver()->newSound(file, VROSoundType::SoundField);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(soundEffect);
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