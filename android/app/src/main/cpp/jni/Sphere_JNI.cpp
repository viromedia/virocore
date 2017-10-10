//
//  Sphere_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//
#include <iostream>
#include <jni.h>
#include <memory>
#include <VROSphere.h>
#include <Viro.h>
#include <VROPlatformUtil.h>
#include "VROMaterial.h"
#include "VRONode.h"
#include "PersistentRef.h"
#include "VROMaterialVisual.h"
#include "VideoTexture_JNI.h"
#include "Node_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_Sphere_##method_name

namespace Sphere{
    inline jlong jptr(std::shared_ptr<VROSphere> ptr) {
        PersistentRef<VROSphere> *persistentRef = new PersistentRef<VROSphere>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROSphere> native(jlong ptr) {
        PersistentRef<VROSphere> *persistentRef = reinterpret_cast<PersistentRef<VROSphere> *>(ptr);
        return persistentRef->get();
    }
}

extern "C" {

JNI_METHOD(jlong, nativeCreateSphere)(JNIEnv *env,
                                      jclass clazz,
                                      jfloat radius,
                                      jint widthSegmentCount,
                                      jint heightSegementCount,
                                      jboolean facesOutward) {
    std::shared_ptr<VROSphere> sphere = VROSphere::createSphere(radius,
                                                                widthSegmentCount,
                                                                heightSegementCount,
                                                                facesOutward);
    return Sphere::jptr(sphere);
}

JNI_METHOD(void, nativeDestroySphere)(JNIEnv *env,
                                        jclass clazz,
                                        jlong nativeNode) {
    delete reinterpret_cast<PersistentRef<VRONode> *>(nativeNode);
}

JNI_METHOD(void, nativeAttachToNode)(JNIEnv *env,
                                     jclass clazz,
                                     jlong nativeSphere,
                                     jlong nativeNode) {
    std::weak_ptr<VROSphere> sphereGeometry_w = Sphere::native(nativeSphere);
    std::weak_ptr<VRONode> node_w = Node::native(nativeNode);

    VROPlatformDispatchAsyncRenderer([sphereGeometry_w, node_w] {
        std::shared_ptr<VROSphere> sphereGeometry = sphereGeometry_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();
        if (sphereGeometry && node) {
            node->setGeometry(sphereGeometry);
        }
    });
}

JNI_METHOD(void, nativeSetVideoTexture)(JNIEnv *env,
                             jobject obj,
                             jlong sphereRef,
                             jlong textureRef) {
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

}  // extern "C"
