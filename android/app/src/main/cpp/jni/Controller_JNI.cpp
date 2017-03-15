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
      Java_com_viro_renderer_jni_ControllerJni_##method_name

extern "C" {

JNI_METHOD(void, nativeSetEventDelegate)(JNIEnv *env,
                                         jobject obj,
                                         jlong render_context_ref,
                                         jlong native_delegate_ref) {
    std::shared_ptr<RenderContext> nativeContext = RenderContext::native(render_context_ref);
    std::shared_ptr<EventDelegate_JNI> delegate = EventDelegate::native(native_delegate_ref);
    VROPlatformDispatchAsyncRenderer([nativeContext, delegate] {
        std::shared_ptr<VROInputPresenter> controllerPresenter
                = nativeContext->getInputController()->getPresenter();
        controllerPresenter->setEventDelegate(delegate);
    });
}

JNI_METHOD(void, nativeEnableReticle)(JNIEnv *env,
                                      jobject obj,
                                      jlong render_context_ref,
                                      jboolean enable) {
    std::shared_ptr<RenderContext> nativeContext = RenderContext::native(render_context_ref);

    VROPlatformDispatchAsyncRenderer([nativeContext, enable] {
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
    std::shared_ptr<RenderContext> nativeContext = RenderContext::native(render_context_ref);

    VROPlatformDispatchAsyncRenderer([nativeContext, enable] {
        std::shared_ptr<VROInputPresenter> controllerPresenter
                = nativeContext->getInputController()->getPresenter();
        controllerPresenter->getRootNode()->setHidden(!enable);
    });
}
/**
 * TODO VIRO-704: Add APIs for custom controls - replacing Obj or adding tooltip support.
 */

}  // extern "C"
