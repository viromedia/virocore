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

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_ViroContext_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateViroContext)(JNIEnv *env,
                                           jobject obj,
                                           jlong renderer_j) {
    std::shared_ptr<ViroContext> context = std::make_shared<ViroContext>(Renderer::native(renderer_j));
    return ViroContext::jptr(context);
}

JNI_METHOD(void, nativeDeleteViroContext)(JNIEnv *env,
                                          jobject obj,
                                          jlong context_j) {
    delete reinterpret_cast<PersistentRef<ViroContext> *>(context_j);
}

JNI_METHOD(void, nativeGetCameraOrientation)(JNIEnv *env,
                                             jobject obj,
                                             jlong context_j,
                                             jobject callback) {
    jweak weakCallback = env->NewWeakGlobalRef(callback);
    std::weak_ptr<ViroContext> context_w = ViroContext::native(context_j);

    VROPlatformDispatchAsyncRenderer([context_w, weakCallback] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject jCallback = env->NewLocalRef(weakCallback);
        if (jCallback == NULL) {
            return;
        }
        std::shared_ptr<ViroContext> helperContext = context_w.lock();
        if (!helperContext) {
            return;
        }

        std::shared_ptr<VRORenderContext> context = helperContext->getContext();
        VROVector3f position = context->getCamera().getPosition();
        VROVector3f rotation = context->getCamera().getRotation().toEuler();
        VROVector3f forward = context->getCamera().getForward();
        VROVector3f up = context->getCamera().getUp();

        VROPlatformCallJavaFunction(jCallback,
                                    "onGetCameraOrientation", "(FFFFFFFFFFFF)V",
                                    position.x, position.y, position.z,
                                    toDegrees(rotation.x), toDegrees(rotation.y), toDegrees(rotation.z),
                                    forward.x, forward.y, forward.z,
                                    up.x, up.y, up.z);
        env->DeleteLocalRef(jCallback);
    });
}

}  // extern "C"
