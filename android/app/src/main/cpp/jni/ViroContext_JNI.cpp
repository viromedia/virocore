//
//  ViroContext_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include <PersistentRef.h>
#include "ViroContext_JNI.h"
#include "VRORenderer_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ViroContext_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateViroContext)(VRO_ARGS
                                             VRO_REF renderer_j) {
    std::shared_ptr<ViroContext> context = std::make_shared<ViroContext>(Renderer::native(renderer_j));
    return ViroContext::jptr(context);
}

VRO_METHOD(void, nativeDeleteViroContext)(VRO_ARGS
                                          VRO_REF context_j) {
    delete reinterpret_cast<PersistentRef<ViroContext> *>(context_j);
}

VRO_METHOD(void, nativeGetCameraOrientation)(VRO_ARGS
                                             VRO_REF context_j,
                                             VRO_OBJECT callback) {
    jweak weakCallback = env->NewWeakGlobalRef(callback);
    std::weak_ptr<ViroContext> context_w = ViroContext::native(context_j);

    VROPlatformDispatchAsyncRenderer([context_w, weakCallback] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        VRO_OBJECT jCallback = env->NewLocalRef(weakCallback);
        if (jCallback == NULL) {
            return;
        }
        std::shared_ptr<ViroContext> helperContext = context_w.lock();
        if (!helperContext) {
            return;
        }

        const VROCamera &camera = helperContext->getCamera();
        VROVector3f position = camera.getPosition();
        VROVector3f rotation = camera.getRotation().toEuler();
        VROVector3f forward = camera.getForward();
        VROVector3f up = camera.getUp();

        VROPlatformCallJavaFunction(jCallback,
                                    "onGetCameraOrientation", "(FFFFFFFFFFFF)V",
                                    position.x, position.y, position.z,
                                    rotation.x, rotation.y, rotation.z,
                                    forward.x, forward.y, forward.z,
                                    up.x, up.y, up.z);
        env->DeleteLocalRef(jCallback);
    });
}

}  // extern "C"
