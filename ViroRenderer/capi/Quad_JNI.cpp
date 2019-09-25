//
//  Quad_JNI.cpp
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

#include "Quad_JNI.h"
#include <VROVideoSurface.h>
#include <VROVideoTextureAVP.h>
#include "VRONode.h"

#include "VROMaterial.h"
#include "VROFrameSynchronizer.h"
#include "VRORenderer_JNI.h"
#include "Node_JNI.h"
#include "ViroContext_JNI.h"
#include "VideoTexture_JNI.h"
#include "Material_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Quad_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Quad_##method_name
#endif


extern "C" {

VRO_METHOD(VRO_REF(VROSurface), nativeCreateQuad)(JNIEnv *env,
                                       jobject object,
                                       jfloat width,
                                       jfloat height,
                                       jfloat u0, jfloat v0,
                                       jfloat u1, jfloat v1) {
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(width, height, u0, v0, u1, v1);
    return VRO_REF_NEW(VROSurface, surface);
}

VRO_METHOD(VRO_REF(VROSurface), nativeCreateQuadFromQuad)(JNIEnv *env,
                                                  jobject object,
                                                  jfloat width,
                                                  jfloat height,
                                                  jfloat u0, jfloat v0,
                                                  jfloat u1, jfloat v1,
                                                  jlong oldSurface) {
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(width, height, u0, v0, u1, v1);
    std::vector<std::shared_ptr<VROMaterial>> materials = VRO_REF_GET(VROSurface, oldSurface)->getMaterials();
    if (materials.size() > 0) {
        surface->setMaterials(materials);
    }

    return VRO_REF_NEW(VROSurface, surface);
}

VRO_METHOD(void, nativeSetWidth)(JNIEnv *env,
                                 jclass clazz,
                                 jlong nativeSurface,
                                 jfloat width) {
    std::weak_ptr<VROSurface> surface_w = VRO_REF_GET(VROSurface, nativeSurface);
    VROPlatformDispatchAsyncRenderer([surface_w, width] {
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface) {
            return;
        }
        surface->setWidth(width);
    });
}

VRO_METHOD(void, nativeSetHeight)(JNIEnv *env,
                                  jclass clazz,
                                  jlong nativeSurface,
                                  jfloat height) {
    std::weak_ptr<VROSurface> surface_w = VRO_REF_GET(VROSurface, nativeSurface);
    VROPlatformDispatchAsyncRenderer([surface_w, height] {
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface) {
            return;
        }
        surface->setHeight(height);
    });
}

VRO_METHOD(void, nativeSetVideoTexture)(JNIEnv *env,
                                        jobject obj,
                                        jlong surfaceRef,
                                        jlong textureRef) {

    std::weak_ptr<VROSurface> surface_w = VRO_REF_GET(VROSurface, surfaceRef);
    std::weak_ptr<VROVideoTexture> videoTexture_w = VRO_REF_GET(VROVideoTexture, textureRef);

    VROPlatformDispatchAsyncRenderer([surface_w, videoTexture_w] {
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface) {
            return;
        }
        std::shared_ptr<VROVideoTexture> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }

        passert (!surface->getMaterials().empty());

        const std::shared_ptr<VROMaterial> &material = surface->getMaterials().front();
        material->getDiffuse().setTexture(videoTexture);
    });
}

VRO_METHOD(void, nativeSetImageTexture)(VRO_ARGS
                                        VRO_REF(VROSurface) surfaceRef,
                                        VRO_REF(VROTexture) textureRef) {
    std::weak_ptr<VROTexture> imageTexture_w = VRO_REF_GET(VROTexture, textureRef);
    std::weak_ptr<VROSurface> surface_w = VRO_REF_GET(VROSurface, surfaceRef);

    VROPlatformDispatchAsyncRenderer([surface_w, imageTexture_w] {
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface) {
            return;
        }
        std::shared_ptr<VROTexture> imageTexture = imageTexture_w.lock();
        if (!imageTexture) {
            return;
        }
        passert (!surface->getMaterials().empty());

        std::vector<std::shared_ptr<VROMaterial>> tempMaterials;
        for (int i = 0; i < surface->getMaterials().size(); i++) {
            // Always copy materials from the material manager, as they may be
            // modified by animations, etc. and we don't want these changes to
            // propagate to the reference material held by the material manager
            tempMaterials.push_back(std::make_shared<VROMaterial>(surface->getMaterials()[i]));
        }

        const std::shared_ptr<VROMaterial> &material = tempMaterials.front();
        material->getDiffuse().setTexture(imageTexture);
        surface->setMaterials(tempMaterials);
    });
}

VRO_METHOD(void, nativeClearMaterial)(JNIEnv *env,
                                      jobject obj,
                                      jlong surfaceRef) {
    std::weak_ptr<VROSurface> surface_w = VRO_REF_GET(VROSurface, surfaceRef);
    VROPlatformDispatchAsyncRenderer([surface_w] {
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface) {
            return;
        }
        surface->setMaterials({ });
    });
}

}  // extern "C"