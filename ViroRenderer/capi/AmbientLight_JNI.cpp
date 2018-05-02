//
// AmbientLight_JNI.cpp
// ViroRenderer
//
// Copyright © 2016 Viro Media. All rights reserved.

#include "VROLight.h"
#include "VRONode.h"
#include "VROPlatformUtil.h"
#include "Node_JNI.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL              \
        Java_com_viro_core_AmbientLight_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type AmbientLight_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROLight), nativeCreateAmbientLight)(VRO_ARGS
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
    return VRO_REF_NEW(VROLight, ambientLight);
}

} // extern "C"