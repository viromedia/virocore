//
//  Surface_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
// THIS FILE IS DEPRECATED. PLEASE MODIFY QUAD_JNI.CPP INSTEAD.
//

#include "Surface_JNI.h"
#include <VROVideoSurface.h>
#include "VRONode.h"
#include "VROMaterial.h"
#include "VROFrameSynchronizer.h"
#include "Node_JNI.h"
#include "ViroContext_JNI.h"
#include "VideoTexture_JNI.h"
#include "Material_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Surface_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Surface_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROSurface), nativeCreateSurface)(VRO_ARGS
                                                     VRO_FLOAT width,
                                                     VRO_FLOAT height,
                                                     VRO_FLOAT u0, VRO_FLOAT v0,
                                                     VRO_FLOAT u1, VRO_FLOAT v1) {
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(width, height, u0, v0, u1, v1);
    return VRO_REF_NEW(VROSurface, surface);
}

VRO_METHOD(VRO_REF(VROSurface), nativeCreateSurfaceFromSurface)(VRO_ARGS
                                                                VRO_FLOAT width,
                                                                VRO_FLOAT height,
                                                                VRO_FLOAT u0, VRO_FLOAT v0,
                                                                VRO_FLOAT u1, VRO_FLOAT v1,
                                                                VRO_REF(VROSurface) oldSurface) {
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(width, height, u0, v0, u1, v1);
    std::vector<std::shared_ptr<VROMaterial>> materials = VRO_REF_GET(VROSurface, oldSurface)->getMaterials();
    if (materials.size() > 0) {
        surface->setMaterials(materials);
    }
    return VRO_REF_NEW(VROSurface, surface);
}

VRO_METHOD(void, nativeDestroySurface)(VRO_ARGS
                                       VRO_REF(VROSurface) nativeSurface) {
    VRO_REF_DELETE(VROSurface, nativeSurface);
}

VRO_METHOD(void, nativeSetWidth)(VRO_ARGS
                                 VRO_REF(VROSurface) nativeSurface,
                                 VRO_FLOAT width) {
    std::weak_ptr<VROSurface> surface_w = VRO_REF_GET(VROSurface, nativeSurface);
    VROPlatformDispatchAsyncRenderer([surface_w, width] {
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface) {
            return;
        }
        surface->setWidth(width);
    });
}

VRO_METHOD(void, nativeSetHeight)(VRO_ARGS
                                  VRO_REF(VROSurface) nativeSurface,
                                  VRO_FLOAT height) {
    std::weak_ptr<VROSurface> surface_w = VRO_REF_GET(VROSurface, nativeSurface);
    VROPlatformDispatchAsyncRenderer([surface_w, height] {
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface) {
            return;
        }
        surface->setHeight(height);
    });
}

VRO_METHOD(void, nativeSetVideoTexture)(VRO_ARGS
                                        VRO_REF(VROSurface) surfaceRef,
                                        VRO_REF(VROVideoTexture) textureRef) {
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

VRO_METHOD(void, nativeClearMaterial)(VRO_ARGS
                                      VRO_REF(VROSurface) surfaceRef) {
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