//
//  SpatialSound_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <memory>
#include "SpatialSound_JNI.h"
#include "VROSound.h"
#include "VROSoundDataGVR.h"
#include "ViroContext_JNI.h"
#include "SoundDelegate_JNI.h"
#include "SoundData_JNI.h"
#include "VROPlatformUtil.h"
#include "VRODriver.h"
#include "VROStringUtil.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_SpatialSound_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type SpatialSound_##method_name
#endif

extern "C" {
VRO_METHOD(VRO_REF, nativeCreateSpatialSound)(VRO_ARGS
                                              VRO_STRING uri_j,
                                              VRO_REF context_j) {
    VRO_METHOD_PREAMBLE;

    std::shared_ptr<ViroContext> context = ViroContext::native(context_j);
    std::string uri = VRO_STRING_STL(uri_j);
    std::shared_ptr<VROSound> soundEffect = context->getDriver()->newSound(uri, VROResourceType::URL, VROSoundType::Spatial);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(soundEffect);
    soundGvr->setDelegate(std::make_shared<SoundDelegate>(obj));
    return SpatialSound::jptr(soundGvr);
}

VRO_METHOD(VRO_REF, nativeCreateSpatialSoundWithData)(VRO_ARGS
                                                      VRO_REF dataRef,
                                                      VRO_REF context_j) {
    VRO_METHOD_PREAMBLE;

    std::shared_ptr<ViroContext> context = ViroContext::native(context_j);
    std::shared_ptr<VROSoundDataGVR> data = SoundData::native(dataRef);

    std::shared_ptr<VROSound> sound = context->getDriver()->newSound(data, VROSoundType::Spatial);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(sound);
    std::shared_ptr<SoundDelegate> delegate = std::make_shared<SoundDelegate>(obj);
    soundGvr->setDelegate(delegate);

    return SpatialSound::jptr(soundGvr);
}

VRO_METHOD(void, nativePlaySpatialSound)(VRO_ARGS VRO_REF nativeRef) {
    SpatialSound::native(nativeRef)->play();
}

VRO_METHOD(void, nativePauseSpatialSound)(VRO_ARGS VRO_REF nativeRef) {
    SpatialSound::native(nativeRef)->pause();
}

VRO_METHOD(void, nativeSetVolume)(VRO_ARGS
                                  VRO_REF nativeRef,
                                  VRO_FLOAT volume) {
    SpatialSound::native(nativeRef)->setVolume(volume);
}

VRO_METHOD(void, nativeSetMuted)(VRO_ARGS
                                 VRO_REF nativeRef,
                                 VRO_BOOL muted) {
    SpatialSound::native(nativeRef)->setMuted(muted);
}

VRO_METHOD(void, nativeSetLoop)(VRO_ARGS
                                VRO_REF nativeRef,
                                VRO_BOOL loop) {
    SpatialSound::native(nativeRef)->setLoop(loop);
}

VRO_METHOD(void, nativeSeekToTime)(VRO_ARGS
                                   VRO_REF nativeRef,
                                   VRO_FLOAT seconds) {
    SpatialSound::native(nativeRef)->seekToTime(seconds);
}

VRO_METHOD(void, nativeSetPosition)(VRO_ARGS
                                    VRO_REF nativeRef,
                                    VRO_FLOAT posX,
                                    VRO_FLOAT posY,
                                    VRO_FLOAT posZ) {
    SpatialSound::native(nativeRef)->setPosition({posX, posY, posZ});
}

VRO_METHOD(void, nativeSetDistanceRolloff)(VRO_ARGS
                                           VRO_REF nativeRef,
                                           VRO_STRING model,
                                           VRO_FLOAT minDistance,
                                           VRO_FLOAT maxDistance) {
    VRO_METHOD_PREAMBLE;
    std::string modelString = VRO_STRING_STL(model);

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

VRO_METHOD(void, nativeDestroySpatialSound)(VRO_ARGS
                                            VRO_REF nativeRef) {
    SpatialSound::native(nativeRef)->setDelegate(nullptr);
    delete reinterpret_cast<PersistentRef<VROSoundGVR> *>(nativeRef);
}

}  // extern "C"