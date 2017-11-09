//
// Created by Raj Advani on 10/24/17.
//

#include "Light_JNI.h"
#include "VROPlatformUtil.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Light_##method_name

extern "C" {

JNI_METHOD(void, nativeDestroyLight)(JNIEnv *env, jclass clazz,
                                     jlong native_light_ref) {
    delete reinterpret_cast<PersistentRef<VROLight> *>(native_light_ref);
}

JNI_METHOD(void, nativeSetColor)(JNIEnv *env,
                                 jclass clazz,
                                 jlong native_light_ref,
                                 jlong color) {
    std::weak_ptr<VROLight> light_w = Light::native(native_light_ref);

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

JNI_METHOD(void, nativeSetIntensity)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_light_ref,
                                     jfloat intensity) {
    std::shared_ptr<VROLight> light = Light::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light, intensity] {
        light->setIntensity(intensity);
    });
}

JNI_METHOD(void, nativeSetInfluenceBitMask)(JNIEnv *env,
                                            jclass clazz,
                                            jlong native_light_ref,
                                            jint bitMask) {
    std::weak_ptr<VROLight> light_w = Light::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, bitMask] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if (!light) {
            return;
        }
        light->setInfluenceBitMask(bitMask);
    });
}

}




