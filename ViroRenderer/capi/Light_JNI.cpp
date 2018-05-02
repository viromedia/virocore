//
// Created by Raj Advani on 10/24/17.
//

#include "Light_JNI.h"
#include "VROPlatformUtil.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Light_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Light_##method_name
#endif

extern "C" {

VRO_METHOD(void, nativeDestroyLight)(VRO_ARGS
                                     VRO_REF(VROLight) native_light_ref) {
    VRO_REF_DELETE(VROLight, native_light_ref);
}

VRO_METHOD(void, nativeSetColor)(VRO_ARGS
                                 VRO_REF(VROLight) native_light_ref,
                                 VRO_LONG color) {
    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);

    VROPlatformDispatchAsyncRenderer([light_w, color] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if (!light) {
            return;
        }
        // Get the color
        float r = ((color >> 16) & 0xFF) / 255.0;
        float g = ((color >> 8) & 0xFF) / 255.0;
        float b = (color & 0xFF) / 255.0;

        VROVector3f vecColor(r, g, b);
        light->setColor(vecColor);
    });
}

VRO_METHOD(void, nativeSetIntensity)(VRO_ARGS
                                     VRO_REF(VROLight) native_light_ref,
                                     VRO_FLOAT intensity) {
    std::shared_ptr<VROLight> light = VRO_REF_GET(VROLight, native_light_ref);
    VROPlatformDispatchAsyncRenderer([light, intensity] {
        light->setIntensity(intensity);
    });
}

VRO_METHOD(void, nativeSetTemperature)(VRO_ARGS
                                       VRO_REF(VROLight) native_light_ref,
                                       VRO_FLOAT temperature) {
    std::shared_ptr<VROLight> light = VRO_REF_GET(VROLight, native_light_ref);
    VROPlatformDispatchAsyncRenderer([light, temperature] {
        light->setTemperature(temperature);
    });
}

VRO_METHOD(void, nativeSetInfluenceBitMask)(VRO_ARGS
                                            VRO_REF(VROLight) native_light_ref,
                                            VRO_INT bitMask) {
    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, bitMask] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if (!light) {
            return;
        }
        light->setInfluenceBitMask(bitMask);
    });
}

}




