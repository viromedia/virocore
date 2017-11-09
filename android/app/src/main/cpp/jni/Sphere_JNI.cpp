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
      Java_com_viro_core_Sphere_##method_name

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
                                      jfloat radius) {
    std::shared_ptr<VROSphere> sphere = std::make_shared<VROSphere>(radius);
    return Sphere::jptr(sphere);
}

JNI_METHOD(jlong, nativeCreateSphereParameterized)(JNIEnv *env,
                                                   jclass clazz,
                                                   jfloat radius,
                                                   jint widthSegmentCount,
                                                   jint heightSegmentCount,
                                                   jboolean facesOutward) {
    std::shared_ptr<VROSphere> sphere = VROSphere::createSphere(radius,
                                                                widthSegmentCount,
                                                                heightSegmentCount,
                                                                facesOutward);
    return Sphere::jptr(sphere);
}

JNI_METHOD(void, nativeDestroySphere)(JNIEnv *env,
                                        jclass clazz,
                                        jlong nativeNode) {
    delete reinterpret_cast<PersistentRef<VRONode> *>(nativeNode);
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

JNI_METHOD(void, nativeSetWidthSegmentCount)(JNIEnv *env,
                                             jobject obj,
                                             jlong jsphere,
                                             jint widthSegmentCount) {

    std::weak_ptr<VROSphere> sphere_w = Sphere::native(jsphere);
    VROPlatformDispatchAsyncRenderer([sphere_w, widthSegmentCount] {
        std::shared_ptr<VROSphere> sphere = sphere_w.lock();
        if (!sphere) {
            return;
        }
        sphere->setWidthSegments(widthSegmentCount);
    });
}

JNI_METHOD(void, nativeSetHeightSegmentCount)(JNIEnv *env,
                                              jobject obj,
                                              jlong jsphere,
                                              jint heightSegmentCount) {
    std::weak_ptr<VROSphere> sphere_w = Sphere::native(jsphere);
    VROPlatformDispatchAsyncRenderer([sphere_w, heightSegmentCount] {
        std::shared_ptr<VROSphere> sphere = sphere_w.lock();
        if (!sphere) {
            return;
        }
        sphere->setHeightSegments(heightSegmentCount);
    });
}

JNI_METHOD(void, nativeSetRadius)(JNIEnv *env,
                                  jobject obj,
                                  jlong jsphere,
                                  jfloat radius) {
    std::weak_ptr<VROSphere> sphere_w = Sphere::native(jsphere);
    VROPlatformDispatchAsyncRenderer([sphere_w, radius] {
        std::shared_ptr<VROSphere> sphere = sphere_w.lock();
        if (!sphere) {
            return;
        }
        sphere->setRadius(radius);
    });
}

JNI_METHOD(void, nativeSetFacesOutward)(JNIEnv *env,
                                       jobject obj,
                                       jlong jsphere,
                                       jboolean facesOutward) {
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
