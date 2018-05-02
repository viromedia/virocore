//
//  SoundField_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <memory>
#include <VROSound.h>
#include <VROSoundGVR.h>
#include "PersistentRef.h"
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

namespace SoundField {
    inline VRO_REF(VROSoundGVR) jptr(std::shared_ptr<VROSoundGVR> ptr) {
        PersistentRef<VROSoundGVR> *persistentRef = new PersistentRef<VROSoundGVR>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROSoundGVR> native(VRO_REF(VROSoundGVR) ptr) {
        PersistentRef<VROSoundGVR> *persistentRef = reinterpret_cast<PersistentRef<VROSoundGVR> *>(ptr);
        return persistentRef->get();
    }
}

extern "C" {

VRO_METHOD(VRO_REF(VROSoundGVR), nativeCreateSoundField)(VRO_ARGS
                                                         VRO_STRING filename,
                                                         VRO_REF(ViroContext) context_j) {
    std::shared_ptr<ViroContext> context = ViroContext::native(context_j);
    std::string file = VRO_STRING_STL(filename);
    std::shared_ptr<VROSound> soundEffect = context->getDriver()->newSound(file, VROResourceType::URL, VROSoundType::SoundField);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(soundEffect);
    soundGvr->setDelegate(std::make_shared<SoundDelegate>(obj));

    return SoundField::jptr(soundGvr);
}

VRO_METHOD(VRO_REF(VROSoundGVR), nativeCreateSoundFieldWithData)(VRO_ARGS
                                                                 VRO_REF(VROSoundDataGVR) dataRef,
                                                                 VRO_REF(ViroContext) context_j) {
    VRO_METHOD_PREAMBLE;

    std::shared_ptr<ViroContext> context = ViroContext::native(context_j);
    std::shared_ptr<VROSoundDataGVR> data = SoundData::native(dataRef);

    std::shared_ptr<VROSound> sound = context->getDriver()->newSound(data, VROSoundType::SoundField);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(sound);
    soundGvr->setDelegate(std::make_shared<SoundDelegate>(obj));

    return SoundField::jptr(soundGvr);
}

VRO_METHOD(void, nativePlaySoundField)(VRO_ARGS
                                       VRO_REF(VROSoundGVR) nativeRef) {
    SoundField::native(nativeRef)->play();
}

VRO_METHOD(void, nativePauseSoundField)(VRO_ARGS
                                        VRO_REF(VROSoundGVR) nativeRef) {
    SoundField::native(nativeRef)->pause();
}

VRO_METHOD(void, nativeSetVolume)(VRO_ARGS
                                  VRO_REF(VROSoundGVR) nativeRef,
                                  VRO_FLOAT volume) {
    SoundField::native(nativeRef)->setVolume(volume);
}

VRO_METHOD(void, nativeSetMuted)(VRO_ARGS
                                 VRO_REF(VROSoundGVR) nativeRef,
                                 VRO_BOOL muted) {
    SoundField::native(nativeRef)->setMuted(muted);
}

VRO_METHOD(void, nativeSetLoop)(VRO_ARGS
                                VRO_REF(VROSoundGVR) nativeRef,
                                VRO_BOOL loop) {
    SoundField::native(nativeRef)->setLoop(loop);
}

VRO_METHOD(void, nativeSeekToTime)(VRO_ARGS
                                   VRO_REF(VROSoundGVR) nativeRef,
                                   VRO_FLOAT seconds) {
    SoundField::native(nativeRef)->seekToTime(seconds);
}

VRO_METHOD(void, nativeSetRotation)(VRO_ARGS
                                    VRO_REF(VROSoundGVR) nativeRef,
                                    VRO_FLOAT rotationRadiansX,
                                    VRO_FLOAT rotationRadiansY,
                                    VRO_FLOAT rotationRadiansZ) {
    SoundField::native(nativeRef)->setRotation({rotationRadiansX, rotationRadiansY, rotationRadiansZ});
}

VRO_METHOD(void, nativeDestroySoundField)(VRO_ARGS
                                          VRO_REF(VROSoundGVR) nativeRef) {
    delete reinterpret_cast<PersistentRef<VROSoundGVR> *>(nativeRef);
}

}  // extern "C"