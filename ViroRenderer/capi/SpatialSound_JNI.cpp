//
//  SpatialSound_JNI.cpp
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
VRO_METHOD(VRO_REF(VROSoundGVR), nativeCreateSpatialSound)(VRO_ARGS
                                                           VRO_STRING uri_j,
                                                           VRO_REF(ViroContext) context_j) {
    VRO_METHOD_PREAMBLE;

    std::shared_ptr<ViroContext> context = VRO_REF_GET(ViroContext, context_j);
    std::string uri = VRO_STRING_STL(uri_j);
    std::shared_ptr<VROSound> soundEffect = context->getDriver()->newSound(uri, VROResourceType::URL, VROSoundType::Spatial);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(soundEffect);
    soundGvr->setDelegate(std::make_shared<SoundDelegate>(obj));
    return VRO_REF_NEW(VROSoundGVR, soundGvr);
}

VRO_METHOD(VRO_REF(VROSoundGVR), nativeCreateSpatialSoundWithData)(VRO_ARGS
                                                                   VRO_REF(VROSoundDataGVR) dataRef,
                                                                   VRO_REF(ViroContext) context_j) {
    VRO_METHOD_PREAMBLE;

    std::shared_ptr<ViroContext> context = VRO_REF_GET(ViroContext, context_j);
    std::shared_ptr<VROSoundDataGVR> data = VRO_REF_GET(VROSoundDataGVR, dataRef);

    std::shared_ptr<VROSound> sound = context->getDriver()->newSound(data, VROSoundType::Spatial);
    std::shared_ptr<VROSoundGVR> soundGvr = std::dynamic_pointer_cast<VROSoundGVR>(sound);
    std::shared_ptr<SoundDelegate> delegate = std::make_shared<SoundDelegate>(obj);
    soundGvr->setDelegate(delegate);

    return VRO_REF_NEW(VROSoundGVR, soundGvr);
}

VRO_METHOD(void, nativePlaySpatialSound)(VRO_ARGS
                                         VRO_REF(VROSoundGVR) nativeRef) {
    VRO_REF_GET(VROSoundGVR, nativeRef)->play();
}

VRO_METHOD(void, nativePauseSpatialSound)(VRO_ARGS
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

VRO_METHOD(void, nativeSetPosition)(VRO_ARGS
                                    VRO_REF(VROSoundGVR) nativeRef,
                                    VRO_FLOAT posX,
                                    VRO_FLOAT posY,
                                    VRO_FLOAT posZ) {
    VRO_REF_GET(VROSoundGVR, nativeRef)->setPosition({posX, posY, posZ});
}

VRO_METHOD(void, nativeSetDistanceRolloff)(VRO_ARGS
                                           VRO_REF(VROSoundGVR) nativeRef,
                                           VRO_STRING model,
                                           VRO_FLOAT minDistance,
                                           VRO_FLOAT maxDistance) {
    VRO_METHOD_PREAMBLE;
    std::string modelString = VRO_STRING_STL(model);

    if (VROStringUtil::strcmpinsensitive(modelString, "none")) {
        VRO_REF_GET(VROSoundGVR, nativeRef)->setDistanceRolloffModel(VROSoundRolloffModel::None,
                                                                 minDistance, maxDistance);
    } else if (VROStringUtil::strcmpinsensitive(modelString, "linear")) {
        VRO_REF_GET(VROSoundGVR, nativeRef)->setDistanceRolloffModel(VROSoundRolloffModel::Linear,
                                                                 minDistance, maxDistance);
    } else if (VROStringUtil::strcmpinsensitive(modelString, "logarithmic")) {
        VRO_REF_GET(VROSoundGVR, nativeRef)->setDistanceRolloffModel(VROSoundRolloffModel::Logarithmic,
                                                                 minDistance, maxDistance);
    }
}

VRO_METHOD(void, nativeDestroySpatialSound)(VRO_ARGS
                                            VRO_REF(VROSoundGVR) nativeRef) {
    VRO_REF_GET(VROSoundGVR, nativeRef)->setDelegate(nullptr);
    VRO_REF_DELETE(VROSoundGVR, nativeRef);
}

}  // extern "C"