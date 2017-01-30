//
//  Sound_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#include <jni.h>
#include <memory>
#include <VRORenderContext.h>
#include <VROSoundGVR.h>
#include "PersistentRef.h"
#include "RenderContext_JNI.h"
#include "SoundDelegate_JNI.h"


#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_SoundJni_##method_name

// TODO: when GVR audio supports the seekToTime, etc, then change the native object to a VROSound.
namespace Sound {
    inline jlong jptr(std::shared_ptr<VROAudioPlayerAndroid> ptr) {
        PersistentRef<VROAudioPlayerAndroid> *persistentRef = new PersistentRef<VROAudioPlayerAndroid>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROAudioPlayerAndroid> native(jlong ptr) {
        PersistentRef<VROAudioPlayerAndroid> *persistentRef = reinterpret_cast<PersistentRef<VROAudioPlayerAndroid> *>(ptr);
        return persistentRef->get();
    }
}

extern "C" {
    /**
     * Since we're using VROAudioPlayerAndroid, there's no difference between the logic for
     * web urls vs local file urls, the Android MediaPlayer handles both.
     */
    JNI_METHOD(jlong, nativeCreateSound)(JNIEnv *env,
                                                 jobject object,
                                                 jstring filename,
                                                 jlong renderContextRef) {
        std::shared_ptr<RenderContext> renderContext = RenderContext::native(renderContextRef);

        const char *cStrFile = env->GetStringUTFChars(filename, NULL);
        std::string file(cStrFile);
        env->ReleaseStringUTFChars(filename, cStrFile);

        // TODO: VIRO-756 do something different for local files
        std::shared_ptr<VROAudioPlayer> player = renderContext->getDriver()->newAudioPlayer(file);
        std::shared_ptr<VROAudioPlayerAndroid> playerAndroid = std::dynamic_pointer_cast<VROAudioPlayerAndroid>(player);
        playerAndroid->setDelegate(std::make_shared<SoundDelegate>(object));

        return Sound::jptr(playerAndroid);
    }

    JNI_METHOD(void, nativePlaySound)(JNIEnv *env, jobject obj,
                                      jlong nativeRef) {
        Sound::native(nativeRef)->play();
    }

    JNI_METHOD(void, nativePauseSound)(JNIEnv *env, jobject obj, jlong nativeRef) {
        Sound::native(nativeRef)->pause();
    }

    JNI_METHOD(void, nativeSetVolume)(JNIEnv *env, jobject obj,
                                      jlong nativeRef,
                                      jfloat volume) {
        Sound::native(nativeRef)->setVolume(volume);
    }

    JNI_METHOD(void, nativeSetMuted)(JNIEnv *env, jobject obj,
                                     jlong nativeRef,
                                     jboolean muted) {
        Sound::native(nativeRef)->setMuted(muted);
    }

    JNI_METHOD(void, nativeSetLoop)(JNIEnv *env, jobject obj,
                                    jlong nativeRef,
                                    jboolean loop) {
        Sound::native(nativeRef)->setLoop(loop);
    }
    JNI_METHOD(void, nativeSeekToTime)(JNIEnv *env, jobject obj,
                                       jlong nativeRef,
                                       jfloat seconds) {
        Sound::native(nativeRef)->seekToTime(seconds);
    }

    JNI_METHOD(void, nativeDestroySound)(JNIEnv *env, jobject obj, jlong nativeRef) {
        delete reinterpret_cast<PersistentRef<VROSoundGVR> *>(nativeRef);
    }

}  // extern "C"