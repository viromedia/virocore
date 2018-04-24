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
#include <VROSoundDataGVR.h>
#include "PersistentRef.h"
#include "ViroContext_JNI.h"
#include "SoundDelegate_JNI.h"
#include "SoundData_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Sound_##method_name
#endif

// TODO: when GVR audio supports the seekToTime, etc, then change the native object to a VROSound.
namespace Sound {
    inline VRO_REF jptr(std::shared_ptr<VROAudioPlayerAndroid> ptr) {
        PersistentRef<VROAudioPlayerAndroid> *persistentRef = new PersistentRef<VROAudioPlayerAndroid>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROAudioPlayerAndroid> native(VRO_REF ptr) {
        PersistentRef<VROAudioPlayerAndroid> *persistentRef = reinterpret_cast<PersistentRef<VROAudioPlayerAndroid> *>(ptr);
        return persistentRef->get();
    }
}

extern "C" {
    /**
     * Since we're using VROAudioPlayerAndroid, there's no difference between the logic for
     * web urls vs local file urls, the Android MediaPlayer handles both.
     */
    VRO_METHOD(VRO_REF, nativeCreateSound)(VRO_ARGS
                                           VRO_STRING filename,
                                           VRO_REF context_j) {
        VROPlatformSetEnv(env); // Invoke in case renderer has not yet initialized
        std::shared_ptr<ViroContext> context = ViroContext::native(context_j);

        std::string file = VROPlatformGetString(filename, env);
        std::shared_ptr<VROAudioPlayer> player = context->getDriver()->newAudioPlayer(file, false);
        std::shared_ptr<VROAudioPlayerAndroid> playerAndroid = std::dynamic_pointer_cast<VROAudioPlayerAndroid>(player);
        playerAndroid->setDelegate(std::make_shared<SoundDelegate>(obj));
        return Sound::jptr(playerAndroid);
    }

    VRO_METHOD(void, nativeSetup)(VRO_ARGS
                                  VRO_REF sound_j) {
        std::shared_ptr<VROAudioPlayerAndroid> player = Sound::native(sound_j);
        player->setup();
    }

    VRO_METHOD(VRO_REF, nativeCreateSoundWithData)(VRO_ARGS
                                                   VRO_REF dataRef,
                                                   VRO_REF context_j) {
        VROPlatformSetEnv(env); // Invoke in case renderer has not yet initialized
        std::shared_ptr<ViroContext> context = ViroContext::native(context_j);
        std::shared_ptr<VROSoundDataGVR> data = SoundData::native(dataRef);

        std::shared_ptr<VROAudioPlayer> player = context->getDriver()->newAudioPlayer(data);
        std::shared_ptr<VROAudioPlayerAndroid> playerAndroid = std::dynamic_pointer_cast<VROAudioPlayerAndroid>(player);
        playerAndroid->setDelegate(std::make_shared<SoundDelegate>(obj));
        playerAndroid->setup();

        return Sound::jptr(playerAndroid);
    }

    VRO_METHOD(void, nativePlaySound)(VRO_ARGS
                                      VRO_REF nativeRef) {
        Sound::native(nativeRef)->play();
    }

    VRO_METHOD(void, nativePauseSound)(VRO_ARGS
                                       VRO_REF nativeRef) {
        Sound::native(nativeRef)->pause();
    }

    VRO_METHOD(void, nativeSetVolume)(VRO_ARGS
                                      VRO_REF nativeRef,
                                      VRO_FLOAT volume) {
        Sound::native(nativeRef)->setVolume(volume);
    }

    VRO_METHOD(void, nativeSetMuted)(VRO_ARGS
                                     VRO_REF nativeRef,
                                     jboolean muted) {
        Sound::native(nativeRef)->setMuted(muted);
    }

    VRO_METHOD(void, nativeSetLoop)(VRO_ARGS
                                    VRO_REF nativeRef,
                                    jboolean loop) {
        Sound::native(nativeRef)->setLoop(loop);
    }
    VRO_METHOD(void, nativeSeekToTime)(VRO_ARGS
                                       VRO_REF nativeRef,
                                       VRO_FLOAT seconds) {
        Sound::native(nativeRef)->seekToTime(seconds);
    }

    VRO_METHOD(void, nativeDestroySound)(VRO_ARGS
                                         VRO_REF nativeRef) {
        delete reinterpret_cast<PersistentRef<VROAudioPlayerAndroid> *>(nativeRef);
    }

}  // extern "C"