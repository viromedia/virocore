//
// SpotLight_JNI.cpp
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
      Java_com_viro_renderer_jni_SpotLightJni_##method_name

namespace SpotLight {
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

JNI_METHOD(jlong, nativeCreateSpotLight)(JNIEnv *env,
                                     jclass clazz,
                                     jlong color,
                                     jfloat attenuationStartDistance,
                                     jfloat attenuationEndDistance,
                                     jfloat positionX,
                                     jfloat positionY,
                                     jfloat positionZ,
                                     jfloat directionX,
                                     jfloat directionY,
                                     jfloat directionZ,
                                     jfloat innerAngle,
                                     jfloat outerAngle) {

    std::shared_ptr<VROLight> spotLight = std::make_shared<VROLight>(VROLightType::Spot);

    // Get the color
    float r = ((color >> 16) & 0xFF) / 255.0;
    float g = ((color >> 8) & 0xFF) / 255.0;
    float b = (color & 0xFF) / 255.0;

    VROVector3f vecColor(r, g, b);
    spotLight->setColor(vecColor);
    spotLight->setAttenuationStartDistance(attenuationStartDistance);
    spotLight->setAttenuationEndDistance(attenuationEndDistance);

    VROVector3f vecPosition(positionX, positionY, positionZ);
    spotLight->setPosition(vecPosition);

    VROVector3f vecDirection(directionX, directionY, directionZ);
    spotLight->setDirection(vecDirection);

    spotLight->setSpotInnerAngle(innerAngle);
    spotLight->setSpotOuterAngle(outerAngle);

    return SpotLight::jptr(spotLight);
}

JNI_METHOD(void, nativeDestroySpotLight)(JNIEnv *env,
                                            jclass clazz,
                                            jlong native_light_ref) {
    delete reinterpret_cast<PersistentRef<VROLight> *>(native_light_ref);
}

JNI_METHOD(void, nativeAddToNode)(JNIEnv *env,
                                  jclass clazz,
                                  jlong native_light_ref,
                                  jlong native_node_ref) {
    std::shared_ptr<VROLight> light = SpotLight::native(native_light_ref);
    std::shared_ptr<VRONode> node = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([light, node] {
        node->addLight(light);
    });
}

JNI_METHOD(void, nativeRemoveFromNode)(JNIEnv *env,
                                       jclass clazz,
                                       jlong native_light_ref,
                                       jlong native_node_ref) {
    std::shared_ptr<VROLight> light = SpotLight::native(native_light_ref);
    std::shared_ptr<VRONode> node = Node::native(native_node_ref);

    VROPlatformDispatchAsyncRenderer([light, node] {
        node->removeLight(light);
    });
}

// Setters

JNI_METHOD(void, nativeSetColor)(JNIEnv *env,
                                 jclass clazz,
                                 jlong native_light_ref,
                                 jlong color) {
    std::shared_ptr<VROLight> light = SpotLight::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light, color] {
        // Get the color
        float r = ((color >> 16) & 0xFF) / 255.0;
        float g = ((color >> 8) & 0xFF) / 255.0;
        float b = (color & 0xFF) / 255.0;

        VROVector3f vecColor(r, g, b);
        light->setColor(vecColor);
    });
}

JNI_METHOD(void, nativeSetAttenuationStartDistance)(JNIEnv *env,
                                                    jclass clazz,
                                                    jlong native_light_ref,
                                                    jfloat attenuationStartDistance) {
    std::shared_ptr<VROLight> light = SpotLight::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light, attenuationStartDistance] {
        light->setAttenuationStartDistance(attenuationStartDistance);
    });
}

JNI_METHOD(void, nativeSetAttenuationEndDistance)(JNIEnv *env,
                                                    jclass clazz,
                                                    jlong native_light_ref,
                                                    jfloat attenuationEndDistance) {
    std::shared_ptr<VROLight> light = SpotLight::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light, attenuationEndDistance] {
        light->setAttenuationEndDistance(attenuationEndDistance);
    });
}

JNI_METHOD(void, nativeSetPosition)(JNIEnv *env,
                                                  jclass clazz,
                                                  jlong native_light_ref,
                                                  jfloat positionX,
                                                  jfloat positionY,
                                                  jfloat positionZ) {
    std::shared_ptr<VROLight> light = SpotLight::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light, positionX, positionY, positionZ] {
        VROVector3f vecPosition(positionX, positionY, positionZ);
        light->setPosition(vecPosition);
    });
}

JNI_METHOD(void, nativeSetDirection)(JNIEnv *env,
                                    jclass clazz,
                                    jlong native_light_ref,
                                    jfloat directionX,
                                    jfloat directionY,
                                    jfloat directionZ) {
    std::shared_ptr<VROLight> light = SpotLight::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light, directionX, directionY, directionZ] {
        VROVector3f vecDirection(directionX, directionY, directionZ);
        light->setDirection(vecDirection);
    });
}

JNI_METHOD(void, nativeSetInnerAngle)(JNIEnv *env,
                                                  jclass clazz,
                                                  jlong native_light_ref,
                                                  jfloat innerAngle) {
    std::shared_ptr<VROLight> light = SpotLight::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light, innerAngle] {
        light->setSpotInnerAngle(innerAngle);
    });
}

JNI_METHOD(void, nativeSetOuterAngle)(JNIEnv *env,
                                                  jclass clazz,
                                                  jlong native_light_ref,
                                                  jfloat outerAngle) {
    std::shared_ptr<VROLight> light = SpotLight::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light, outerAngle] {
        light->setSpotOuterAngle(outerAngle);
    });
}
} // extern "C"