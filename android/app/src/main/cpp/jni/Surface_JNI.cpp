//
//  Surface_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "Surface_JNI.h"
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
      Java_com_viro_core_Surface_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateSurface)(VRO_ARGS
                                         jfloat width,
                                         jfloat height,
                                         jfloat u0, jfloat v0,
                                         jfloat u1, jfloat v1) {
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(width, height, u0, v0, u1, v1);
    return Surface::jptr(surface);
}

VRO_METHOD(VRO_REF, nativeCreateSurfaceFromSurface)(VRO_ARGS
                                                    jfloat width,
                                                    jfloat height,
                                                    jfloat u0, jfloat v0,
                                                    jfloat u1, jfloat v1,
                                                    VRO_REF oldSurface) {
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(width, height, u0, v0, u1, v1);
    std::vector<std::shared_ptr<VROMaterial>> materials = Surface::native(oldSurface)->getMaterials();
    if (materials.size() > 0) {
        surface->setMaterials(materials);
    }
    return Surface::jptr(surface);
}

VRO_METHOD(void, nativeDestroySurface)(VRO_ARGS
                                       VRO_REF nativeSurface) {
    delete reinterpret_cast<PersistentRef<VROSurface> *>(nativeSurface);
}

VRO_METHOD(void, nativeSetWidth)(VRO_ARGS
                                 VRO_REF nativeSurface,
                                 jfloat width) {
    std::weak_ptr<VROSurface> surface_w = Surface::native(nativeSurface);
    VROPlatformDispatchAsyncRenderer([surface_w, width] {
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface) {
            return;
        }
        surface->setWidth(width);
    });
}

VRO_METHOD(void, nativeSetHeight)(VRO_ARGS
                                  VRO_REF nativeSurface,
                                  jfloat height) {
    std::weak_ptr<VROSurface> surface_w = Surface::native(nativeSurface);
    VROPlatformDispatchAsyncRenderer([surface_w, height] {
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface) {
            return;
        }
        surface->setHeight(height);
    });
}

VRO_METHOD(void, nativeSetVideoTexture)(VRO_ARGS
                                        VRO_REF surfaceRef,
                                        VRO_REF textureRef) {
    std::weak_ptr<VROSurface> surface_w = Surface::native(surfaceRef);
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

VRO_METHOD(void, nativeSetImageTexture)(VRO_ARGS
                                        VRO_REF surfaceRef,
                                        VRO_REF textureRef) {
    std::weak_ptr<VROTexture> imageTexture_w = Texture::native(textureRef);
    std::weak_ptr<VROSurface> surface_w = Surface::native(surfaceRef);

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

VRO_METHOD(void, nativeClearMaterial)(VRO_ARGS
                                      VRO_REF surfaceRef) {
    std::weak_ptr<VROSurface> surface_w = Surface::native(surfaceRef);
    VROPlatformDispatchAsyncRenderer([surface_w] {
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface) {
            return;
        }
        surface->setMaterials({ });
    });
}

}  // extern "C"