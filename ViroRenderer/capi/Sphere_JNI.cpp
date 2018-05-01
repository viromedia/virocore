//
//  Sphere_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <iostream>
#include <memory>
#include <VROSphere.h>
#include <VROPlatformUtil.h>
#include "VROMaterial.h"
#include "VRONode.h"
#include "PersistentRef.h"
#include "VROMaterialVisual.h"
#include "VideoTexture_JNI.h"
#include "Node_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Sphere_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Sphere_##method_name
#endif

namespace Sphere{
    inline VRO_REF jptr(std::shared_ptr<VROSphere> ptr) {
        PersistentRef<VROSphere> *persistentRef = new PersistentRef<VROSphere>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROSphere> native(VRO_REF ptr) {
        PersistentRef<VROSphere> *persistentRef = reinterpret_cast<PersistentRef<VROSphere> *>(ptr);
        return persistentRef->get();
    }
}

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateSphere)(VRO_ARGS
                                        VRO_FLOAT radius) {
    std::shared_ptr<VROSphere> sphere = std::make_shared<VROSphere>(radius);
    return Sphere::jptr(sphere);
}

VRO_METHOD(VRO_REF, nativeCreateSphereParameterized)(VRO_ARGS
                                                     VRO_FLOAT radius,
                                                     VRO_INT widthSegmentCount,
                                                     VRO_INT heightSegmentCount,
                                                     VRO_BOOL facesOutward) {
    std::shared_ptr<VROSphere> sphere = VROSphere::createSphere(radius,
                                                                widthSegmentCount,
                                                                heightSegmentCount,
                                                                facesOutward);
    return Sphere::jptr(sphere);
}

VRO_METHOD(void, nativeDestroySphere)(VRO_ARGS
                                      VRO_REF nativeNode) {
    delete reinterpret_cast<PersistentRef<VRONode> *>(nativeNode);
}

VRO_METHOD(void, nativeSetVideoTexture)(VRO_ARGS
                                        VRO_REF sphereRef,
                                        VRO_REF textureRef) {
    std::weak_ptr<VROSphere> sphere_w = Sphere::native(sphereRef);
    std::weak_ptr<VROVideoTexture> videoTexture_w = VideoTexture::native(textureRef);

    VROPlatformDispatchAsyncRenderer([sphere_w, videoTexture_w] {
        std::shared_ptr<VROSphere> sphere = sphere_w.lock();
        std::shared_ptr<VROVideoTexture> videoTexture = videoTexture_w.lock();
        if (!sphere || !videoTexture) {
            return;
        }

        std::shared_ptr<VROMaterial> material;
        if (sphere->getMaterials().size() > 0) {
            // If there's an existing material, make a copy of that so that
            // we can shift existing materials to the end, and align Video Texture
            // materials to be the first on the array.
            material = std::make_shared<VROMaterial>(sphere->getMaterials()[0]);
        } else {
            material = std::make_shared<VROMaterial>();
        }

        material->setWritesToDepthBuffer(false);
        material->setReadsFromDepthBuffer(false);
        material->getDiffuse().setTexture(videoTexture);
        sphere->setMaterials({ material });
    });
}

VRO_METHOD(void, nativeSetWidthSegmentCount)(VRO_ARGS
                                             VRO_REF jsphere,
                                             VRO_INT widthSegmentCount) {

    std::weak_ptr<VROSphere> sphere_w = Sphere::native(jsphere);
    VROPlatformDispatchAsyncRenderer([sphere_w, widthSegmentCount] {
        std::shared_ptr<VROSphere> sphere = sphere_w.lock();
        if (!sphere) {
            return;
        }
        sphere->setWidthSegments(widthSegmentCount);
    });
}

VRO_METHOD(void, nativeSetHeightSegmentCount)(VRO_ARGS
                                              VRO_REF jsphere,
                                              VRO_INT heightSegmentCount) {
    std::weak_ptr<VROSphere> sphere_w = Sphere::native(jsphere);
    VROPlatformDispatchAsyncRenderer([sphere_w, heightSegmentCount] {
        std::shared_ptr<VROSphere> sphere = sphere_w.lock();
        if (!sphere) {
            return;
        }
        sphere->setHeightSegments(heightSegmentCount);
    });
}

VRO_METHOD(void, nativeSetRadius)(VRO_ARGS
                                  VRO_REF jsphere,
                                  VRO_FLOAT radius) {
    std::weak_ptr<VROSphere> sphere_w = Sphere::native(jsphere);
    VROPlatformDispatchAsyncRenderer([sphere_w, radius] {
        std::shared_ptr<VROSphere> sphere = sphere_w.lock();
        if (!sphere) {
            return;
        }
        sphere->setRadius(radius);
    });
}

VRO_METHOD(void, nativeSetFacesOutward)(VRO_ARGS
                                        VRO_REF jsphere,
                                        VRO_BOOL facesOutward) {
    std::weak_ptr<VROSphere> sphere_w = Sphere::native(jsphere);
    VROPlatformDispatchAsyncRenderer([sphere_w, facesOutward] {
        std::shared_ptr<VROSphere> sphere = sphere_w.lock();
        if (!sphere) {
            return;
        }
        sphere->setFacesOutward(facesOutward);
    });
}

}  // extern "C"
