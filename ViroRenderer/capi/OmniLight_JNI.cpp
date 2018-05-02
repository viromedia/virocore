//
// OmniLight_JNI.cpp
// ViroRenderer
//
// Copyright © 2016 Viro Media. All rights reserved.

#include "VROLight.h"
#include "VRONode.h"
#include "VROPlatformUtil.h"
#include "Node_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_OmniLight_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type OmniLight_##method_name
#endif

extern "C" {


VRO_METHOD(VRO_REF(VROLight), nativeCreateOmniLight)(VRO_ARGS
                                                     VRO_LONG color,
                                                     VRO_FLOAT intensity,
                                                     VRO_FLOAT attenuationStartDistance,
                                                     VRO_FLOAT attenuationEndDistance,
                                                     VRO_FLOAT positionX,
                                                     VRO_FLOAT positionY,
                                                     VRO_FLOAT positionZ) {

    std::shared_ptr<VROLight> omniLight = std::make_shared<VROLight>(VROLightType::Omni);

    // Get the color
    float r = ((color >> 16) & 0xFF) / 255.0;
    float g = ((color >> 8) & 0xFF) / 255.0;
    float b = (color & 0xFF) / 255.0;

    VROVector3f vecColor(r, g, b);
    omniLight->setColor(vecColor);
    omniLight->setIntensity(intensity);
    omniLight->setAttenuationStartDistance(attenuationStartDistance);
    omniLight->setAttenuationEndDistance(attenuationEndDistance);

    VROVector3f vecPosition(positionX, positionY, positionZ);
    omniLight->setPosition(vecPosition);

    return VRO_REF_NEW(VROLight, omniLight);
}

// Setters

VRO_METHOD(void, nativeSetAttenuationStartDistance)(VRO_ARGS
                                                    VRO_REF(VROLight) native_light_ref,
                                                    VRO_FLOAT attenuationStartDistance) {
    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, attenuationStartDistance] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if (!light) {
            return;
        }
        light->setAttenuationStartDistance(attenuationStartDistance);
    });
}

VRO_METHOD(void, nativeSetAttenuationEndDistance)(VRO_ARGS
                                                  VRO_REF(VROLight) native_light_ref,
                                                  VRO_FLOAT attenuationEndDistance) {
    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, attenuationEndDistance] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if (!light) {
            return;
        }
        light->setAttenuationEndDistance(attenuationEndDistance);
    });
}

VRO_METHOD(void, nativeSetPosition)(VRO_ARGS
                                    VRO_REF(VROLight) native_light_ref,
                                    VRO_FLOAT positionX,
                                    VRO_FLOAT positionY,
                                    VRO_FLOAT positionZ) {
    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, positionX, positionY, positionZ] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if (!light) {
            return;
        }
        VROVector3f vecPosition(positionX, positionY, positionZ);
        light->setPosition(vecPosition);
    });
}
}