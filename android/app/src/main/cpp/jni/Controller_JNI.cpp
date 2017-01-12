//
//  Controller_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include <VROInputPresenterDaydream.h>
#include "VROMaterial.h"
#include "PersistentRef.h"
#include "VROInputPresenterDaydream.h"
#include "Node_JNI.h"
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
    std::shared_ptr<VROInputPresenter> controllerPresenter
            = nativeContext->getInputController()->getPresenter();
    controllerPresenter->setEventDelegate(EventDelegate::native(native_delegate_ref));
}

JNI_METHOD(void, nativeEnableReticle)(JNIEnv *env,
                                      jobject obj,
                                      jlong render_context_ref,
                                      jboolean enable) {
    std::shared_ptr<RenderContext> nativeContext = RenderContext::native(render_context_ref);
    std::shared_ptr<VROInputPresenter> controllerPresenter
            = nativeContext->getInputController()->getPresenter();
    controllerPresenter->getReticle()->setEnabled(enable);
}

/**
 * TODO VIRO-704: Add APIs for custom controls - replacing Obj or adding tooltip support.
 */

}  // extern "C"
