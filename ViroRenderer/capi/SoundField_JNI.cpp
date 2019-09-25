//
//  SoundField_JNI.cpp
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
#include <VROSound.h>
#include <VROSoundGVR.h>
#include "ViroContext_JNI.h"
#include "SoundDelegate_JNI.h"
#include "SoundData_JNI.h"
#include "VRODriver.h"
#include "VROPlatformUtil.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_SoundField_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type SoundField_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROSoundGVR), nativeCreateSoundField)(VRO_ARGS
                                                         VRO_STRING filename,
                                                         VRO_REF(ViroContext) context_j) {
    std::shared_ptr<ViroContext> context = VRO_REF_GET(ViroContext, context_j);
    std::string file = VRO_STRING_STL(filename);
    std::shared_ptr<VROSound> soundEffect = context->getDriver()->newSound(file, VROResourceType::URL, VROSoundType::SoundField);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(soundEffect);
    soundGvr->setDelegate(std::make_shared<SoundDelegate>(obj));

    return VRO_REF_NEW(VROSoundGVR, soundGvr);
}

VRO_METHOD(VRO_REF(VROSoundGVR), nativeCreateSoundFieldWithData)(VRO_ARGS
                                                                 VRO_REF(VROSoundDataGVR) dataRef,
                                                                 VRO_REF(ViroContext) context_j) {
    VRO_METHOD_PREAMBLE;

    std::shared_ptr<ViroContext> context = VRO_REF_GET(ViroContext, context_j);
    std::shared_ptr<VROSoundDataGVR> data = VRO_REF_GET(VROSoundDataGVR, dataRef);

    std::shared_ptr<VROSound> sound = context->getDriver()->newSound(data, VROSoundType::SoundField);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(sound);
    soundGvr->setDelegate(std::make_shared<SoundDelegate>(obj));

    return VRO_REF_NEW(VROSoundGVR, soundGvr);
}

VRO_METHOD(void, nativePlaySoundField)(VRO_ARGS
                                       VRO_REF(VROSoundGVR) nativeRef) {
    VRO_REF_GET(VROSoundGVR, nativeRef)->play();
}

VRO_METHOD(void, nativePauseSoundField)(VRO_ARGS
                                        VRO_REF(VROSoundGVR) nativeRef) {
    VRO_REF_GET(VROSoundGVR, nativeRef)->pause();
}

VRO_METHOD(void, nativeSetVolume)(VRO_ARGS
                                  VRO_REF(VROSoundGVR) nativeRef,
                                  VRO_FLOAT volume) {
    VRO_REF_GET(VROSoundGVR, nativeRef)->setVolume(volume);
}

VRO_METHOD(void, nativeSetMuted)(VRO_ARGS
                                 VRO_REF(VROSoundGVR) nativeRef,
                                 VRO_BOOL muted) {
    VRO_REF_GET(VROSoundGVR, nativeRef)->setMuted(muted);
}

VRO_METHOD(void, nativeSetLoop)(VRO_ARGS
                                VRO_REF(VROSoundGVR) nativeRef,
                                VRO_BOOL loop) {
    VRO_REF_GET(VROSoundGVR, nativeRef)->setLoop(loop);
}

VRO_METHOD(void, nativeSeekToTime)(VRO_ARGS
                                   VRO_REF(VROSoundGVR) nativeRef,
                                   VRO_FLOAT seconds) {
    VRO_REF_GET(VROSoundGVR, nativeRef)->seekToTime(seconds);
}

VRO_METHOD(void, nativeSetRotation)(VRO_ARGS
                                    VRO_REF(VROSoundGVR) nativeRef,
                                    VRO_FLOAT rotationRadiansX,
                                    VRO_FLOAT rotationRadiansY,
                                    VRO_FLOAT rotationRadiansZ) {
    VRO_REF_GET(VROSoundGVR, nativeRef)->setRotation({rotationRadiansX, rotationRadiansY, rotationRadiansZ});
}

VRO_METHOD(void, nativeDestroySoundField)(VRO_ARGS
                                          VRO_REF(VROSoundGVR) nativeRef) {
    VRO_REF_DELETE(VROSoundGVR, nativeRef);
}

}  // extern "C"