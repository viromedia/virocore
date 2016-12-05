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

}  // extern "C"
