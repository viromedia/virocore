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
                                                jfloat intensity,
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
    directionalLight->setIntensity(intensity);

    VROVector3f vecDirection(directionX, directionY, directionZ);
    directionalLight->setDirection(vecDirection);

    return DirectionalLight::jptr(directionalLight);
}

JNI_METHOD(void, nativeDestroyDirectionalLight)(JNIEnv *env,
                                         jclass clazz,
                                         jlong native_light_ref) {
    delete reinterpret_cast<PersistentRef<VROLight> *>(native_light_ref);
}

JNI_METHOD(void, nativeAddToNode)(JNIEnv *env,
                                  jclass clazz,
                                  jlong native_light_ref,
                                  jlong native_node_ref) {
    std::weak_ptr<VROLight> light_w = DirectionalLight::native(native_light_ref);
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);

    VROPlatformDispatchAsyncRenderer([light_w, node_w] {
        std::shared_ptr<VROLight> light = light_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();
        if (light && node) {
            node->addLight(light);
        }
    });
}

JNI_METHOD(void, nativeRemoveFromNode)(JNIEnv *env,
                                       jclass clazz,
                                       jlong native_light_ref,
                                       jlong native_node_ref) {
    std::weak_ptr<VROLight> light_w = DirectionalLight::native(native_light_ref);
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);

    VROPlatformDispatchAsyncRenderer([light_w, node_w] {
        std::shared_ptr<VROLight> light = light_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();
        if (light && node) {
            node->removeLight(light);
        }
    });
}

// Setters

JNI_METHOD(void, nativeSetColor)(JNIEnv *env,
                                 jclass clazz,
                                 jlong native_light_ref,
                                 jlong color) {
    std::weak_ptr<VROLight> light_w = DirectionalLight::native(native_light_ref);

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
    std::shared_ptr<VROLight> light = DirectionalLight::native(native_light_ref);

    VROPlatformDispatchAsyncRenderer([light, intensity] {
        light->setIntensity(intensity);
    });
}

JNI_METHOD(void, nativeSetDirection)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_light_ref,
                                     jfloat directionX,
                                     jfloat directionY,
                                     jfloat directionZ) {
    std::weak_ptr<VROLight> light_w = DirectionalLight::native(native_light_ref);

    VROPlatformDispatchAsyncRenderer([light_w, directionX, directionY, directionZ] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if (!light) {
            return;
        }
        VROVector3f vecDirection(directionX, directionY, directionZ);
        light->setDirection(vecDirection);
    });
}


JNI_METHOD(void, nativeSetCastsShadow)(JNIEnv *env,
                                       jclass clazz,
                                       jlong native_light_ref,
                                       jboolean castsShadow) {
    std::weak_ptr<VROLight> light_w = DirectionalLight::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, castsShadow] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if (!light) {
            return;
        }
        light->setCastsShadow(castsShadow);
    });
}

JNI_METHOD(void, nativeSetInfluenceBitMask)(JNIEnv *env,
                                            jclass clazz,
                                            jlong native_light_ref,
                                            jint bitMask) {

    std::weak_ptr<VROLight> light_w = DirectionalLight::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, bitMask] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if (!light) {
            return;
        }
        light->setInfluenceBitMask(bitMask);
    });
}
//shadow properties
JNI_METHOD(void, nativeSetShadowOrthographicSize)(JNIEnv *env,
                                                 jclass clazz,
                                                 jlong native_light_ref,
                                                 jfloat size) {

    std::weak_ptr<VROLight> light_w = DirectionalLight::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, size] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        light->setShadowOrthographicSize(size);
    });
}

JNI_METHOD(void, nativeSetShadowOrthographicPosition)(JNIEnv *env,
                                                  jclass clazz,
                                                  jlong native_light_ref,
                                                  jfloat posX,
                                                  jfloat posY,
                                                  jfloat posZ) {

    std::weak_ptr<VROLight> light_w = DirectionalLight::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, posX, posY, posZ] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        VROVector3f vecPosition(posX, posY, posZ);
        light->setPosition(vecPosition);
    });
}

JNI_METHOD(void, nativeSetShadowMapSize)(JNIEnv *env,
                                         jclass clazz,
                                         jlong native_light_ref,
                                         jint size) {

    std::weak_ptr<VROLight> light_w = DirectionalLight::native(native_light_ref);

    VROPlatformDispatchAsyncRenderer([light_w, size] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        light->setShadowMapSize(size);
    });
}

JNI_METHOD(void, nativeSetShadowOpacity)(JNIEnv *env,
                                         jclass clazz,
                                         jlong native_light_ref,
                                         float opacity) {

    std::weak_ptr<VROLight> light_w = DirectionalLight::native(native_light_ref);

    VROPlatformDispatchAsyncRenderer([light_w, opacity] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        light->setShadowOpacity(opacity);
    });
}

JNI_METHOD(void, nativeSetShadowBias)(JNIEnv *env,
                                      jclass clazz,
                                      jlong native_light_ref,
                                      jfloat bias) {

    std::weak_ptr<VROLight> light_w = DirectionalLight::native(native_light_ref);

    VROPlatformDispatchAsyncRenderer([light_w, bias] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        light->setShadowBias(bias);
    });
}


JNI_METHOD(void, nativeSetShadowNearZ)(JNIEnv *env,
                                      jclass clazz,
                                      jlong native_light_ref,
                                      jfloat shadowNearZ) {

    std::weak_ptr<VROLight> light_w = DirectionalLight::native(native_light_ref);

    VROPlatformDispatchAsyncRenderer([light_w, shadowNearZ] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        light->setShadowNearZ(shadowNearZ);
    });
}

JNI_METHOD(void, nativeSetShadowFarZ)(JNIEnv *env,
                                       jclass clazz,
                                       jlong native_light_ref,
                                       jfloat shadowFarZ) {


    std::weak_ptr<VROLight> light_w = DirectionalLight::native(native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, shadowFarZ] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        light->setShadowFarZ(shadowFarZ);
    });
}

} // extern "C"