//
//  Camera_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
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

#include "Camera_JNI.h"
#include <iostream>
#include <memory>
#include <VROPlatformUtil.h>
#include "VRONodeCamera.h"
#include "VROCamera.h"
#include "VROStringUtil.h"
#include "Node_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Camera_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Camera_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VRONodeCamera), nativeCreateCamera)(VRO_NO_ARGS) {
    std::shared_ptr<VRONodeCamera> node = std::make_shared<VRONodeCamera>();
    return VRO_REF_NEW(VRONodeCamera, node);
}

VRO_METHOD(void, nativeDestroyCamera)(VRO_ARGS
                                      VRO_REF(VRONodeCamera) native_node_ref) {
    VRO_REF_DELETE(VRONodeCamera, native_node_ref);
}

VRO_METHOD(void, nativeSetPosition)(VRO_ARGS
                                    VRO_REF(VRONodeCamera) nativeCamera, VRO_FLOAT x, VRO_FLOAT y, VRO_FLOAT z) {
    std::weak_ptr<VRONodeCamera> camera_w = VRO_REF_GET(VRONodeCamera, nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, x, y, z] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            camera->setPosition(VROVector3f(x, y, z));
        }
    });
}

VRO_METHOD(void, nativeSetRotationEuler)(VRO_ARGS
                                         VRO_REF(VRONodeCamera) nativeCamera, VRO_FLOAT x, VRO_FLOAT y, VRO_FLOAT z) {
    std::weak_ptr<VRONodeCamera> camera_w = VRO_REF_GET(VRONodeCamera, nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, x, y, z] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            VROQuaternion quaternion = { x, y, z };
            camera->setBaseRotation(quaternion);
        }
    });
}

VRO_METHOD(void, nativeSetRotationQuaternion)(VRO_ARGS
                                              VRO_REF(VRONodeCamera) nativeCamera, VRO_FLOAT x, VRO_FLOAT y, VRO_FLOAT z, VRO_FLOAT w) {
    std::weak_ptr<VRONodeCamera> camera_w = VRO_REF_GET(VRONodeCamera, nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, x, y, z, w] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            VROQuaternion quaternion = { x, y, z, w };
            camera->setBaseRotation(quaternion);
        }
    });
}

VRO_METHOD(void, nativeSetRotationType)(VRO_ARGS
                                        VRO_REF(VRONodeCamera) nativeCamera,
                                        VRO_STRING rotationType) {
    VRO_METHOD_PREAMBLE;

    VROCameraRotationType type;
    if (VROStringUtil::strcmpinsensitive(VRO_STRING_STL(rotationType), "orbit")) {
        type = VROCameraRotationType::Orbit;
    } else {
        // default rotation type is standard.
        type = VROCameraRotationType::Standard;
    }

    std::weak_ptr<VRONodeCamera> camera_w = VRO_REF_GET(VRONodeCamera, nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, type] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            camera->setRotationType(type);
        }
    });
}

VRO_METHOD(void, nativeSetOrbitFocalPoint)(VRO_ARGS
                                           VRO_REF(VRONodeCamera) nativeCamera, VRO_FLOAT x, VRO_FLOAT y, VRO_FLOAT z) {
    std::weak_ptr<VRONodeCamera> camera_w = VRO_REF_GET(VRONodeCamera, nativeCamera);
    VROPlatformDispatchAsyncRenderer([camera_w, x, y, z] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            camera->setOrbitFocalPoint(VROVector3f(x, y, z));
        }
    });
}

VRO_METHOD(void, nativeSetFieldOfView)(VRO_ARGS
                                       VRO_REF(VRONodeCamera) camera_j, VRO_FLOAT fov) {
    std::weak_ptr<VRONodeCamera> camera_w = VRO_REF_GET(VRONodeCamera, camera_j);
    VROPlatformDispatchAsyncRenderer([camera_w, fov] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        if (camera) {
            camera->setFieldOfViewY(fov);
        }
    });
}

VRO_METHOD(void, nativeSetRefNodeToCopyRotation)(VRO_ARGS VRO_REF(VRONodeCamera) camera_j, VRO_REF(VRONode) node_j) {
    std::weak_ptr<VRONodeCamera> camera_w = VRO_REF_GET(VRONodeCamera, camera_j);
    VROPlatformDispatchAsyncRenderer([camera_w, node_j] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        std::shared_ptr<VRONode> childNode = VRO_REF_GET(VRONode, node_j);
        if (camera) {
            camera->setRefNodeToCopyRotation(childNode);
        }
    });
}

} // extern "C"