//
//  DirectionalLight_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
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

#include "VROLight.h"
#include "VRONode.h"
#include "VROPlatformUtil.h"
#include "Node_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_DirectionalLight_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type DirectionalLight_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROLight), nativeCreateDirectionalLight)(VRO_ARGS
                                                            VRO_LONG color,
                                                            VRO_FLOAT intensity,
                                                            VRO_FLOAT directionX,
                                                            VRO_FLOAT directionY,
                                                            VRO_FLOAT directionZ) {

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

    return VRO_REF_NEW(VROLight, directionalLight);
}

// Setters

VRO_METHOD(void, nativeSetDirection)(VRO_ARGS
                                     VRO_REF(VROLight) native_light_ref,
                                     VRO_FLOAT directionX,
                                     VRO_FLOAT directionY,
                                     VRO_FLOAT directionZ) {
    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);

    VROPlatformDispatchAsyncRenderer([light_w, directionX, directionY, directionZ] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if (!light) {
            return;
        }
        VROVector3f vecDirection(directionX, directionY, directionZ);
        light->setDirection(vecDirection);
    });
}


VRO_METHOD(void, nativeSetCastsShadow)(VRO_ARGS
                                       VRO_REF(VROLight) native_light_ref,
                                       VRO_BOOL castsShadow) {
    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, castsShadow] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if (!light) {
            return;
        }
        light->setCastsShadow(castsShadow);
    });
}

VRO_METHOD(void, nativeSetShadowOrthographicSize)(VRO_ARGS
                                                  VRO_REF(VROLight) native_light_ref,
                                                  VRO_FLOAT size) {

    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, size] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        light->setShadowOrthographicSize(size);
    });
}

VRO_METHOD(void, nativeSetShadowOrthographicPosition)(VRO_ARGS
                                                      VRO_REF(VROLight) native_light_ref,
                                                      VRO_FLOAT posX,
                                                      VRO_FLOAT posY,
                                                      VRO_FLOAT posZ) {

    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, posX, posY, posZ] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        VROVector3f vecPosition(posX, posY, posZ);
        light->setPosition(vecPosition);
    });
}

VRO_METHOD(void, nativeSetShadowMapSize)(VRO_ARGS
                                         VRO_REF(VROLight) native_light_ref,
                                         VRO_INT size) {

    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);

    VROPlatformDispatchAsyncRenderer([light_w, size] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        light->setShadowMapSize(size);
    });
}

VRO_METHOD(void, nativeSetShadowOpacity)(VRO_ARGS
                                         VRO_REF(VROLight) native_light_ref,
                                         float opacity) {

    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);

    VROPlatformDispatchAsyncRenderer([light_w, opacity] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        light->setShadowOpacity(opacity);
    });
}

VRO_METHOD(void, nativeSetShadowBias)(VRO_ARGS
                                      VRO_REF(VROLight) native_light_ref,
                                      VRO_FLOAT bias) {

    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);

    VROPlatformDispatchAsyncRenderer([light_w, bias] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        light->setShadowBias(bias);
    });
}


VRO_METHOD(void, nativeSetShadowNearZ)(VRO_ARGS
                                       VRO_REF(VROLight) native_light_ref,
                                       VRO_FLOAT shadowNearZ) {

    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);

    VROPlatformDispatchAsyncRenderer([light_w, shadowNearZ] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        light->setShadowNearZ(shadowNearZ);
    });
}

VRO_METHOD(void, nativeSetShadowFarZ)(VRO_ARGS
                                      VRO_REF(VROLight) native_light_ref,
                                      VRO_FLOAT shadowFarZ) {


    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, shadowFarZ] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if(!light) {
            return;
        }
        light->setShadowFarZ(shadowFarZ);
    });
}

} // extern "C"