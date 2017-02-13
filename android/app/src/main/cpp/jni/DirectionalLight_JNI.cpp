//
// DirectionalLight_JNI.cpp
// ViroRenderer
//
// Copyright © 2016 Viro Media. All rights reserved.

#include <jni.h>
#include <VROLight.h>
#include <PersistentRef.h>
#include <VRONode.h>
#include <VROPlatformUtil.h>
#include "Node_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_DirectionalLightJni_##method_name

namespace DirectionalLight {
    inline jlong jptr(std::shared_ptr<VROLight> shared_node) {
        PersistentRef<VROLight> *native_light = new PersistentRef<VROLight>(shared_node);
        return reinterpret_cast<intptr_t>(native_light);
    }

    inline std::shared_ptr<VROLight> native(jlong ptr) {
        PersistentRef<VROLight> *persistentBox = reinterpret_cast<PersistentRef<VROLight> *>(ptr);
        return persistentBox->get();
    }
}

extern "C" {

JNI_METHOD(jlong, nativeCreateDirectionalLight)(JNIEnv *env,
                                         jclass clazz,
                                         jlong color,
                                         jfloat directionX,
                                         jfloat directionY,
                                         jfloat directionZ) {

    std::shared_ptr<VROLight> directionalLight = std::make_shared<VROLight>(VROLightType::Directional);

    // Get the color
    float r = ((color >> 16) & 0xFF) / 255.0;
    float g = ((color >> 8) & 0xFF) / 255.0;
    float b = (color & 0xFF) / 255.0;

    VROVector3f vecColor(r, g, b);
    directionalLight->setColor(vecColor);

    VROVector3f vecDirection(directionX, directionY, directionZ);
    directionalLight->setDirection(vecDirection);

    return DirectionalLight::jptr(directionalLight);
}

JNI_METHOD(void, nativeDestroyDirectionalLight)(JNIEnv *env,
                                         jclass clazz,
                                         jlong native_light_ref) {
    VROPlatformDispatchAsyncRenderer([native_light_ref] {
        delete reinterpret_cast<PersistentRef<VROLight> *>(native_light_ref);
    });
}

JNI_METHOD(void, nativeAddToNode)(JNIEnv *env,
                                  jclass clazz,
                                  jlong native_light_ref,
                                  jlong native_node_ref) {
    VROPlatformDispatchAsyncRenderer([native_light_ref, native_node_ref] {
        std::shared_ptr<VROLight> light = DirectionalLight::native(native_light_ref);
        Node::native(native_node_ref)->addLight(light);
    });
}

JNI_METHOD(void, nativeRemoveFromNode)(JNIEnv *env,
                                       jclass clazz,
                                       jlong native_light_ref,
                                       jlong native_node_ref) {
    VROPlatformDispatchAsyncRenderer([native_light_ref, native_node_ref] {
        std::shared_ptr<VROLight> light = DirectionalLight::native(native_light_ref);
        Node::native(native_node_ref)->removeLight(light);
    });
}

// Setters

JNI_METHOD(void, nativeSetColor)(JNIEnv *env,
                                 jclass clazz,
                                 jlong native_light_ref,
                                 jlong color) {
    VROPlatformDispatchAsyncRenderer([native_light_ref, color] {
        std::shared_ptr<VROLight> light = DirectionalLight::native(native_light_ref);

        // Get the color
        float r = ((color >> 16) & 0xFF) / 255.0;
        float g = ((color >> 8) & 0xFF) / 255.0;
        float b = (color & 0xFF) / 255.0;

        VROVector3f vecColor(r, g, b);

        light->setColor(vecColor);
    });
}

JNI_METHOD(void, nativeSetDirection)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_light_ref,
                                     jfloat directionX,
                                     jfloat directionY,
                                     jfloat directionZ) {
    VROPlatformDispatchAsyncRenderer([native_light_ref, directionX, directionY, directionZ] {
        std::shared_ptr<VROLight> light = DirectionalLight::native(native_light_ref);
        VROVector3f vecDirection(directionX, directionY, directionZ);
        light->setDirection(vecDirection);
    });
}

} // extern "C"