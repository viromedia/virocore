
//
//  Quad_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//


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

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Quad_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateQuad)(JNIEnv *env,
                                       jobject object,
                                       jfloat width,
                                       jfloat height,
                                       jfloat u0, jfloat v0,
                                       jfloat u1, jfloat v1) {
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(width, height, u0, v0, u1, v1);
    return Quad::jptr(surface);
}

JNI_METHOD(jlong, nativeCreateQuadFromQuad)(JNIEnv *env,
                                                  jobject object,
                                                  jfloat width,
                                                  jfloat height,
                                                  jfloat u0, jfloat v0,
                                                  jfloat u1, jfloat v1,
                                                  jlong oldSurface) {
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(width, height, u0, v0, u1, v1);
    std::vector<std::shared_ptr<VROMaterial>> materials = Quad::native(oldSurface)->getMaterials();
    if (materials.size() > 0) {
        surface->setMaterials(materials);
    }
    return Quad::jptr(surface);
}

JNI_METHOD(void, nativeDestroyQuad)(JNIEnv *env,
                                       jclass clazz,
                                       jlong nativeSurface) {
    delete reinterpret_cast<PersistentRef<VROSurface> *>(nativeSurface);
}

JNI_METHOD(void, nativeSetWidth)(JNIEnv *env,
                                 jclass clazz,
                                 jlong nativeSurface,
                                 jfloat width) {
    std::weak_ptr<VROSurface> surface_w = Quad::native(nativeSurface);
    VROPlatformDispatchAsyncRenderer([surface_w, width] {
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface) {
            return;
        }
        surface->setWidth(width);
    });
}

JNI_METHOD(void, nativeSetHeight)(JNIEnv *env,
                                  jclass clazz,
                                  jlong nativeSurface,
                                  jfloat height) {
    std::weak_ptr<VROSurface> surface_w = Quad::native(nativeSurface);
    VROPlatformDispatchAsyncRenderer([surface_w, height] {
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface) {
            return;
        }
        surface->setHeight(height);
    });
}

JNI_METHOD(void, nativeSetVideoTexture)(JNIEnv *env,
                                        jobject obj,
                                        jlong surfaceRef,
                                        jlong textureRef) {
    std::weak_ptr<VROSurface> surface_w = Quad::native(surfaceRef);
    std::weak_ptr<VROVideoTexture> videoTexture_w = VideoTexture::native(textureRef);

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

JNI_METHOD(void, nativeSetImageTexture)(JNIEnv *env,
                                        jobject obj,
                                        jlong surfaceRef,
                                        jlong textureRef) {
    std::weak_ptr<VROTexture> imageTexture_w = Texture::native(textureRef);
    std::weak_ptr<VROSurface> surface_w = Quad::native(surfaceRef);

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

        const std::shared_ptr<VROMaterial> &material = surface->getMaterials().front();
        material->getDiffuse().setTexture(imageTexture);
    });
}

JNI_METHOD(void, nativeClearMaterial)(JNIEnv *env,
                                      jobject obj,
                                      jlong surfaceRef) {
    std::weak_ptr<VROSurface> surface_w = Quad::native(surfaceRef);
    VROPlatformDispatchAsyncRenderer([surface_w] {
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface) {
            return;
        }
        surface->setMaterials({ });
    });
}

}  // extern "C"