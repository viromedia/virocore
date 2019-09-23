//
//  Sound_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <memory>
#include "VRORenderContext.h"
#include "VROSoundGVR.h"
#include "VROSoundDataGVR.h"
#include "ViroContext_JNI.h"
#include "SoundDelegate_JNI.h"
#include "SoundData_JNI.h"
#include "VROPlatformUtil.h"
#include "VRODriver.h"
#include "VROAudioPlayer.h"

#if VRO_PLATFORM_ANDROID
#include "VROAudioPlayerAndroid.h"

#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Sound_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Sound_##method_name
#endif

extern "C" {

    /**
     * If we're using VROAudioPlayerAndroid, there's no difference between the logic for
     * web urls vs local file urls, the Android MediaPlayer handles both.
     */
    VRO_METHOD(VRO_REF(VROAudioPlayer), nativeCreateSound)(VRO_ARGS
                                                           VRO_STRING filename,
                                                           VRO_REF(ViroContext) context_j) {
        VRO_METHOD_PREAMBLE;
        VROPlatformSetEnv(env); // Invoke in case renderer has not yet initialized
        std::shared_ptr<ViroContext> context = VRO_REF_GET(ViroContext, context_j);

        std::string file = VRO_STRING_STL(filename);
        std::shared_ptr<VROAudioPlayer> player = context->getDriver()->newAudioPlayer(file, false);
        player->setDelegate(std::make_shared<SoundDelegate>(obj));
        return VRO_REF_NEW(VROAudioPlayer, player);
    }

    VRO_METHOD(void, nativeSetup)(VRO_ARGS
                                  VRO_REF(VROAudioPlayer) sound_j) {
        std::shared_ptr<VROAudioPlayer> player = VRO_REF_GET(VROAudioPlayer, sound_j);
        player->setup();
    }

    VRO_METHOD(VRO_REF(VROAudioPlayer), nativeCreateSoundWithData)(VRO_ARGS
                                                                   VRO_REF(VROSoundDataGVR) dataRef,
                                                                   VRO_REF(ViroContext) context_j) {
        VRO_METHOD_PREAMBLE;
        VROPlatformSetEnv(env); // Invoke in case renderer has not yet initialized
        std::shared_ptr<ViroContext> context = VRO_REF_GET(ViroContext, context_j);
        std::shared_ptr<VROSoundDataGVR> data = VRO_REF_GET(VROSoundDataGVR, dataRef);

        std::shared_ptr<VROAudioPlayer> player = context->getDriver()->newAudioPlayer(data);
        player->setDelegate(std::make_shared<SoundDelegate>(obj));
        player->setup();

        return VRO_REF_NEW(VROAudioPlayer, player);
    }

    VRO_METHOD(void, nativePlaySound)(VRO_ARGS
                                      VRO_REF(VROAudioPlayer) nativeRef) {
        VRO_REF_GET(VROAudioPlayer, nativeRef)->play();
    }

    VRO_METHOD(void, nativePauseSound)(VRO_ARGS
                                       VRO_REF(VROAudioPlayer) nativeRef) {
        VRO_REF_GET(VROAudioPlayer, nativeRef)->pause();
    }

    VRO_METHOD(void, nativeSetVolume)(VRO_ARGS
                                      VRO_REF(VROAudioPlayer) nativeRef,
                                      VRO_FLOAT volume) {
        VRO_REF_GET(VROAudioPlayer, nativeRef)->setVolume(volume);
    }

    VRO_METHOD(void, nativeSetMuted)(VRO_ARGS
                                     VRO_REF(VROAudioPlayer) nativeRef,
                                     VRO_BOOL muted) {
        VRO_REF_GET(VROAudioPlayer, nativeRef)->setMuted(muted);
    }

    VRO_METHOD(void, nativeSetLoop)(VRO_ARGS
                                    VRO_REF(VROAudioPlayer) nativeRef,
                                    VRO_BOOL loop) {
        VRO_REF_GET(VROAudioPlayer, nativeRef)->setLoop(loop);
    }
    VRO_METHOD(void, nativeSeekToTime)(VRO_ARGS
                                       VRO_REF(VROAudioPlayer) nativeRef,
                                       VRO_FLOAT seconds) {
        VRO_REF_GET(VROAudioPlayer, nativeRef)->seekToTime(seconds);
    }

    VRO_METHOD(void, nativeDestroySound)(VRO_ARGS
                                         VRO_REF(VROAudioPlayer) nativeRef) {
        VRO_REF_DELETE(VROAudioPlayer, nativeRef);
    }

}  // extern "C"