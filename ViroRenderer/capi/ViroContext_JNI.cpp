//
//  ViroContext_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <memory>
#include "PersistentRef.h"
#include "ViroContext_JNI.h"
#include "VROPlatformUtil.h"
#include "VROVector3f.h"
#include "VROCamera.h"

#if VRO_PLATFORM_ANDROID
#include "VRORenderer_JNI.h"
#include "ViroContextAndroid_JNI.h"

#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ViroContext_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type ViroContext_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(ViroContext), nativeCreateViroContext)(VRO_ARGS
                                                          VRO_REF(VROSceneRenderer) renderer_j) {

    std::shared_ptr<ViroContext> context;
#if VRO_PLATFORM_ANDROID
    context = std::make_shared<ViroContextAndroid>(Renderer::native(renderer_j));
#else
    // TODO wasm
#endif
    return ViroContext::jptr(context);
}

VRO_METHOD(void, nativeDeleteViroContext)(VRO_ARGS
                                          VRO_REF(ViroContext) context_j) {
    delete reinterpret_cast<PersistentRef<ViroContext> *>(context_j);
}

VRO_METHOD(void, nativeGetCameraOrientation)(VRO_ARGS
                                             VRO_REF(ViroContext) context_j,
                                             VRO_OBJECT callback) {
    VRO_WEAK weakCallback = VRO_NEW_WEAK_GLOBAL_REF(callback);
    std::weak_ptr<ViroContext> context_w = ViroContext::native(context_j);

    VROPlatformDispatchAsyncRenderer([context_w, weakCallback] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT jCallback = VRO_NEW_LOCAL_REF(weakCallback);
        if (VRO_IS_OBJECT_NULL(jCallback)) {
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

        VROPlatformCallHostFunction(jCallback,
                                    "onGetCameraOrientation", "(FFFFFFFFFFFF)V",
                                    position.x, position.y, position.z,
                                    rotation.x, rotation.y, rotation.z,
                                    forward.x, forward.y, forward.z,
                                    up.x, up.y, up.z);
        VRO_DELETE_LOCAL_REF(jCallback);
    });
}

}  // extern "C"
