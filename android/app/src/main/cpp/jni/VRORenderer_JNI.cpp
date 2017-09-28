//
//  VRORenderer_JNI.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include <PersistentRef.h>
#include <VROCamera.h>

#include "arcore/ARCore_JNI.h"

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "VROSceneRendererGVR.h"
#include "VROSceneRendererOVR.h"
#include "VROSceneRendererARCore.h"
#include "VROPlatformUtil.h"
#include "VROSample.h"
#include "Node_JNI.h"
#include "VROSceneController.h"
#include "VRORenderer_JNI.h"
#include "VROReticle.h"
#include "SceneController_JNI.h"
#include "object.hpp"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_RendererJni_##method_name

extern "C" {

// The renderer test runs VROSample.cpp, for fast prototyping when working
// on renderer features (no bridge integration). Do not check-in with this
// flag true!
static const bool kRunRendererTest = true;
static std::shared_ptr<VROSample> sample;

JNI_METHOD(jlong, nativeCreateRendererGVR)(JNIEnv *env, jclass clazz,
                                           jobject class_loader,
                                           jobject android_context,
                                           jobject asset_mgr,
                                           jobject platform_util,
                                           jlong native_gvr_api) {
    std::shared_ptr<gvr::AudioApi> gvrAudio = std::make_shared<gvr::AudioApi>();
    gvrAudio->Init(env, android_context, class_loader, GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    VROPlatformSetEnv(env, android_context, asset_mgr, platform_util);

    gvr_context *gvrContext = reinterpret_cast<gvr_context *>(native_gvr_api);
    std::shared_ptr<VROSceneRenderer> renderer
            = std::make_shared<VROSceneRendererGVR>(gvrContext, gvrAudio);
    return Renderer::jptr(renderer);
}

JNI_METHOD(jlong, nativeCreateRendererOVR)(JNIEnv *env, jclass clazz,
                                           jobject class_loader,
                                           jobject android_context,
                                           jobject view,
                                           jobject activity,
                                           jobject asset_mgr,
                                           jobject platform_util) {
    std::shared_ptr<gvr::AudioApi> gvrAudio = std::make_shared<gvr::AudioApi>();
    gvrAudio->Init(env, android_context, class_loader, GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    VROPlatformSetEnv(env, android_context, asset_mgr, platform_util);

    std::shared_ptr<VROSceneRenderer> renderer
            = std::make_shared<VROSceneRendererOVR>(gvrAudio, view, activity, env);
    return Renderer::jptr(renderer);
}

JNI_METHOD(jlong, nativeCreateRendererARCore)(JNIEnv *env, jclass clazz,
                                              jobject class_loader,
                                              jobject android_context,
                                              jobject view,
                                              jni::Object<arcore::Session> session,
                                              jobject asset_mgr,
                                              jobject platform_util) {
    std::shared_ptr<gvr::AudioApi> gvrAudio = std::make_shared<gvr::AudioApi>();
    gvrAudio->Init(env, android_context, class_loader, GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    VROPlatformSetEnv(env, android_context, asset_mgr, platform_util);

    std::shared_ptr<VROSceneRenderer> renderer
            = std::make_shared<VROSceneRendererARCore>(gvrAudio, session);
    return Renderer::jptr(renderer);
}

JNI_METHOD(void, nativeDestroyRenderer)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_renderer) {
    Renderer::native(native_renderer)->onDestroy();
    VROThreadRestricted::unsetThread(VROThreadName::Renderer);

    delete reinterpret_cast<PersistentRef<VROSceneRenderer> *>(native_renderer);
}

JNI_METHOD(void, nativeInitializeGl)(JNIEnv *env,
                                     jobject obj,
                                     jlong native_renderer) {

    VROThreadRestricted::setThread(VROThreadName::Renderer, pthread_self());
    std::shared_ptr<VROSceneRenderer> sceneRenderer = Renderer::native(native_renderer);

    if (kRunRendererTest) {
        sample = std::make_shared<VROSample>();
        sceneRenderer->setRenderDelegate(sample);
        sceneRenderer->setSceneController(
                sample->loadShadowScene(sceneRenderer->getDriver()));
    }
    sceneRenderer->initGL();
}

JNI_METHOD(void, nativeDrawFrame)(JNIEnv *env,
                                  jobject obj,
                                  jlong native_renderer) {
    Renderer::native(native_renderer)->onDrawFrame();
}

JNI_METHOD (void, nativeOnKeyEvent)(JNIEnv * env,
                                    jobject obj,
                                    jlong native_renderer,
                                    int keyCode,
                                    int action ){
    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(native_renderer);
    VROPlatformDispatchAsyncRenderer([renderer_w, keyCode, action] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }
        renderer->onKeyEvent(keyCode, action);
    });
}

JNI_METHOD(void, nativeOnTouchEvent)(JNIEnv *env,
                                       jobject obj,
                                       jlong native_renderer,
                                       jint onTouchAction,
                                           float xPos,
                                           float yPos) {
    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(native_renderer);
    VROPlatformDispatchAsyncRenderer([renderer_w, onTouchAction, xPos, yPos] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }
        renderer->onTouchEvent(onTouchAction, xPos, yPos);
    });
}

JNI_METHOD(void, nativeSetVRModeEnabled)(JNIEnv *env,
                                         jobject obj,
                                         jlong nativeRenderer, jboolean enabled) {
    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(nativeRenderer);
    VROPlatformDispatchAsyncRenderer([renderer_w, enabled] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }
        renderer->setVRModeEnabled(enabled);
    });
}

JNI_METHOD(void, nativeOnStart)(JNIEnv *env,
                                jobject obj,
                                jlong native_renderer) {
        Renderer::native(native_renderer)->onStart();
}

JNI_METHOD(void, nativeOnPause)(JNIEnv *env,
                                jobject obj,
                                jlong native_renderer) {
        Renderer::native(native_renderer)->onPause();
}

JNI_METHOD(void, nativeOnResume)(JNIEnv *env,
                                 jobject obj,
                                 jlong native_renderer) {
        Renderer::native(native_renderer)->onResume();
}

JNI_METHOD(void, nativeOnStop)(JNIEnv *env,
                                jobject obj,
                                jlong native_renderer) {
        Renderer::native(native_renderer)->onStop();
}

JNI_METHOD(void, nativeSetSceneController)(JNIEnv *env,
                                           jobject obj,
                                           jlong native_renderer,
                                           jlong native_scene_controller_ref) {
    if (kRunRendererTest) {
        return;
    }

    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(native_renderer);
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(native_scene_controller_ref);

    VROPlatformDispatchAsyncRenderer([renderer_w, sceneController_w] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        if (!sceneController) {
            return;
        }
        renderer->setSceneController(sceneController);
    });
}

JNI_METHOD(void, nativeSetSceneControllerWithAnimation)(JNIEnv *env,
                                                        jobject obj,
                                                        jlong native_renderer,
                                                        jlong native_scene_controller_ref,
                                                        jfloat duration) {
    if (kRunRendererTest) {
        return;
    }

    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(native_renderer);
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(native_scene_controller_ref);

    VROPlatformDispatchAsyncRenderer([renderer_w, sceneController_w, duration] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        if (!sceneController) {
            return;
        }
        renderer->setSceneController(sceneController, duration, VROTimingFunctionType::EaseOut);
    });
}

JNI_METHOD(void, nativeSetPointOfView)(JNIEnv *env,
                                       jobject obj,
                                       jlong native_renderer,
                                       jlong native_node_ref) {
    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(native_renderer);
    std::shared_ptr<VRONode> node;
    if (native_node_ref != 0) {
        node = Node::native(native_node_ref);
    }

    VROPlatformDispatchAsyncRenderer([renderer_w, node] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }

        if (!node) {
            renderer->getRenderer()->setPointOfView(nullptr);
        }
        else {
            renderer->getRenderer()->setPointOfView(node);
        }
    });
}

JNI_METHOD(void, nativeOnSurfaceCreated)(JNIEnv *env,
                                         jobject obj,
                                         jobject surface,
                                         jlong native_renderer) {

    Renderer::native(native_renderer)->onSurfaceCreated(surface);
}

JNI_METHOD(void, nativeOnSurfaceChanged)(JNIEnv *env,
                                         jobject obj,
                                         jobject surface,
                                         jint width,
                                         jint height,
                                         jlong native_renderer) {
    Renderer::native(native_renderer)->onSurfaceChanged(surface, width, height);
}

JNI_METHOD(void, nativeOnSurfaceDestroyed)(JNIEnv *env,
                                jobject obj,
                                jlong native_renderer) {
    Renderer::native(native_renderer)->onSurfaceDestroyed();
}

JNI_METHOD(jstring, nativeGetHeadset)(JNIEnv *env, jobject obj, jlong nativeRenderer) {
    std::string headset = Renderer::native(nativeRenderer)->getRenderer()->getInputController()->getHeadset();
    return env->NewStringUTF(headset.c_str());
}

JNI_METHOD(jstring, nativeGetController)(JNIEnv *env, jobject obj, jlong nativeRenderer) {
    std::string controller = Renderer::native(nativeRenderer)->getRenderer()->getInputController()->getController();
    return env->NewStringUTF(controller.c_str());
}

JNI_METHOD(void, nativeSetDebugHUDEnabled)(JNIEnv *env,
                                           jobject obj,
                                           jlong native_renderer,
                                           jboolean enabled) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    renderer->getRenderer()->setDebugHUDEnabled(enabled);
}

JNI_METHOD(void, nativeSetSuspended)(JNIEnv *env,
                                        jobject obj,
                                        jlong native_renderer,
                                        jboolean suspend_renderer) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    renderer->setSuspended(suspend_renderer);
}

// This function is OVR only!
JNI_METHOD(void, nativeRecenterTracking)(JNIEnv *env,
                                         jobject obj,
                                         jlong native_renderer) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    std::shared_ptr<VROSceneRendererOVR> ovrRenderer = std::dynamic_pointer_cast<VROSceneRendererOVR>(renderer);
    ovrRenderer->recenterTracking();
}

}  // extern "C"
