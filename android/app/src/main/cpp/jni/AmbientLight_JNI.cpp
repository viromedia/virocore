//
// AmbientLight_JNI.cpp
// ViroRenderer
//
// Copyright © 2016 Viro Media. All rights reserved.

#include <VROLight.h>
#include <PersistentRef.h>
#include <VRONode.h>
#include <VROPlatformUtil.h>
#include "Node_JNI.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL              \
        Java_com_viro_core_AmbientLight_##method_name
#endif

namespace AmbientLight {
    inline VRO_REF jptr(std::shared_ptr<VROLight> shared_node) {
        PersistentRef<VROLight> *native_light = new PersistentRef<VROLight>(shared_node);
        return reinterpret_cast<intptr_t>(native_light);
    }

    inline std::shared_ptr<VROLight> native(VRO_REF ptr) {
        PersistentRef<VROLight> *persistentBox = reinterpret_cast<PersistentRef<VROLight> *>(ptr);
        return persistentBox->get();
    }
}

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateAmbientLight)(VRO_ARGS
                                              VRO_LONG color,
                                              VRO_FLOAT intensity) {
    std::shared_ptr<VROLight> ambientLight = std::make_shared<VROLight>(VROLightType::Ambient);

    // Get the color
    float r = ((color >> 16) & 0xFF) / 255.0;
    float g = ((color >> 8) & 0xFF) / 255.0;
    float b = (color & 0xFF) / 255.0;

    VROVector3f vecColor(r, g, b);
    ambientLight->setColor(vecColor);
    ambientLight->setIntensity(intensity);
    return AmbientLight::jptr(ambientLight);
}

} // extern "C"