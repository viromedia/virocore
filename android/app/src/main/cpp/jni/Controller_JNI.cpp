//
//  Controller_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include <VROInputPresenterDaydream.h>
#include "RenderContext_JNI.h"
#include "EventDelegate_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_Controller_##method_name

extern "C" {

JNI_METHOD(void, nativeSetEventDelegate)(JNIEnv *env,
                                         jobject obj,
                                         jlong render_context_ref,
                                         jlong native_delegate_ref) {
    std::weak_ptr<RenderContext> nativeContext_w = RenderContext::native(render_context_ref);
    std::weak_ptr<EventDelegate_JNI> delegate_w = EventDelegate::native(native_delegate_ref);

    VROPlatformDispatchAsyncRenderer([nativeContext_w, delegate_w] {
        std::shared_ptr<RenderContext> nativeContext = nativeContext_w.lock();
        std::shared_ptr<EventDelegate_JNI> delegate = delegate_w.lock();

        if (nativeContext && delegate) {
            std::shared_ptr<VROInputPresenter> controllerPresenter
                    = nativeContext->getInputController()->getPresenter();
            controllerPresenter->setEventDelegate(delegate);
        }
    });
}

JNI_METHOD(void, nativeEnableReticle)(JNIEnv *env,
                                      jobject obj,
                                      jlong render_context_ref,
                                      jboolean enable) {
    std::weak_ptr<RenderContext> nativeContext_w = RenderContext::native(render_context_ref);

    VROPlatformDispatchAsyncRenderer([nativeContext_w, enable] {
        std::shared_ptr<RenderContext> nativeContext = nativeContext_w.lock();
        if (!nativeContext) {
            return;
        }

        std::shared_ptr<VROInputPresenter> controllerPresenter
                = nativeContext->getInputController()->getPresenter();
        std::shared_ptr<VROReticle> reticle = controllerPresenter->getReticle();
        if (reticle != nullptr) {
            reticle->setEnabled(enable);
        }
    });
}

JNI_METHOD(void, nativeEnableController)(JNIEnv *env,
                                      jobject obj,
                                      jlong render_context_ref,
                                      jboolean enable) {
    std::weak_ptr<RenderContext> nativeContext_w = RenderContext::native(render_context_ref);

    VROPlatformDispatchAsyncRenderer([nativeContext_w, enable] {
        std::shared_ptr<RenderContext> nativeContext = nativeContext_w.lock();
        if (!nativeContext) {
            return;
        }

        std::shared_ptr<VROInputPresenter> controllerPresenter
                = nativeContext->getInputController()->getPresenter();
        controllerPresenter->getRootNode()->setHidden(!enable);
    });
}

JNI_METHOD(void, nativeGetControllerForwardVectorAsync)(JNIEnv *env,
                                          jobject obj,
                                          jlong native_render_context_ref,
                                          jobject callback) {
    jweak weakCallback = env->NewWeakGlobalRef(callback);
    std::weak_ptr<RenderContext> helperContext_w = RenderContext::native(native_render_context_ref);

    VROPlatformDispatchAsyncApplication([helperContext_w, weakCallback] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject jCallback = env->NewLocalRef(weakCallback);
        if (jCallback == NULL) {
            return;
        }
        std::shared_ptr<RenderContext> helperContext = helperContext_w.lock();
        if (!helperContext) {
            return;
        }
        VROVector3f position = helperContext->getInputController()->getPresenter()->getLastKnownForward();
        VROPlatformCallJavaFunction(jCallback,
                                    "onGetForwardVector", "(FFF)V", position.x, position.y, position.z);
        env->DeleteLocalRef(jCallback);
        env->DeleteWeakGlobalRef(weakCallback);
    });
}
/**
 * TODO VIRO-704: Add APIs for custom controls - replacing Obj or adding tooltip support.
 */

}  // extern "C"
