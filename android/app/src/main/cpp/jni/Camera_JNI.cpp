//
//  Camera_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "Camera_JNI.h"
#include <iostream>
#include <jni.h>
#include <memory>
#include <VROPlatformUtil.h>
#include "PersistentRef.h"
#include "VRONodeCamera.h"
#include "VROCamera.h"
#include "VROStringUtil.h"
#include "Node_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Camera_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateCamera)(VRO_NO_ARGS) {
    std::shared_ptr<VRONodeCamera> node = std::make_shared<VRONodeCamera>();
    return Camera::jptr(node);
}

VRO_METHOD(void, nativeDestroyCamera)(VRO_ARGS
                                     VRO_REF native_node_ref) {
    delete reinterpret_cast<PersistentRef<VRONodeCamera> *>(native_node_ref);
}

VRO_METHOD(void, nativeSetPosition)(VRO_ARGS
                                    VRO_REF nativeCamera, VRO_FLOAT x, VRO_FLOAT y, VRO_FLOAT z) {
    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, x, y, z] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            camera->setPosition(VROVector3f(x, y, z));
        }
    });
}

VRO_METHOD(void, nativeSetRotationEuler)(VRO_ARGS
                                         VRO_REF nativeCamera, VRO_FLOAT x, VRO_FLOAT y, VRO_FLOAT z) {
    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, x, y, z] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            VROQuaternion quaternion = { x, y, z };
            camera->setBaseRotation(quaternion);
        }
    });
}

VRO_METHOD(void, nativeSetRotationQuaternion)(VRO_ARGS
                                              VRO_REF nativeCamera, VRO_FLOAT x, VRO_FLOAT y, VRO_FLOAT z, VRO_FLOAT w) {
    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, x, y, z, w] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            VROQuaternion quaternion = { x, y, z, w };
            camera->setBaseRotation(quaternion);
        }
    });
}

VRO_METHOD(void, nativeSetRotationType)(VRO_ARGS
                                        VRO_REF nativeCamera,
                                        jstring rotationType) {
    VROCameraRotationType type;
    if (VROStringUtil::strcmpinsensitive(VROPlatformGetString(rotationType, env), "orbit")) {
        type = VROCameraRotationType::Orbit;
    } else {
        // default rotation type is standard.
        type = VROCameraRotationType::Standard;
    }

    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, type] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            camera->setRotationType(type);
        }
    });
}

VRO_METHOD(void, nativeSetOrbitFocalPoint)(VRO_ARGS
                                           VRO_REF nativeCamera, VRO_FLOAT x, VRO_FLOAT y, VRO_FLOAT z) {
    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, x, y, z] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            camera->setOrbitFocalPoint(VROVector3f(x, y, z));
        }
    });
}

VRO_METHOD(void, nativeSetFieldOfView)(VRO_ARGS
                                       VRO_REF camera_j, VRO_FLOAT fov) {
    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(camera_j);
    VROPlatformDispatchAsyncRenderer([camera_w, fov] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            camera->setFieldOfViewY(fov);
        }
    });
}

} // extern "C"