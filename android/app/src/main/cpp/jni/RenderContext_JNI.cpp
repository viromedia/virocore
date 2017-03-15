//
//  RenderContext_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include <PersistentRef.h>
#include "RenderContext_JNI.h"
#include "VRORenderer_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_RenderContextJni_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateRenderContext)(JNIEnv *env,
                                             jobject obj,
                                             jlong native_renderer) {
    std::shared_ptr<RenderContext> rendererContext
            = std::make_shared<RenderContext>(Renderer::native(native_renderer));
    return RenderContext::jptr(rendererContext);
}

JNI_METHOD(void, nativeDeleteRenderContext)(JNIEnv *env,
                                            jobject obj,
                                            jlong native_render_context_ref) {
    delete reinterpret_cast<PersistentRef<RenderContext> *>(native_render_context_ref);
}

JNI_METHOD(void, nativeGetCameraPosition)(JNIEnv *env,
                                                 jobject obj,
                                                 jlong native_render_context_ref,
                                                 jobject callback) {
    jweak weakCallback = env->NewWeakGlobalRef(callback);
    std::shared_ptr<RenderContext> helperContext = RenderContext::native(native_render_context_ref);

    VROPlatformDispatchAsyncRenderer([helperContext, weakCallback] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject jCallback = env->NewLocalRef(weakCallback);
        if (jCallback == NULL) {
            return;
        }

        std::shared_ptr<VRORenderContext> context = helperContext->getContext();
        VROVector3f position = context->getCamera().getPosition();

        VROPlatformCallJavaFunction(jCallback,
                                    "onGetCameraPosition", "(FFF)V", position.x, position.y, position.z);
        env->DeleteLocalRef(jCallback);
    });
}

}  // extern "C"
