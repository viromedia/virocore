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
#include <VROARHitTestResult.h>
#include <VROFrameListener.h>
#include "arcore/ARCore_Native.h"
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "VROProjector.h"
#include "VROSceneRendererGVR.h"
#include "VROSceneRendererOVR.h"
#include "VROSceneRendererSceneView.h"
#include "VROPlatformUtil.h"
#include "VROSample.h"
#include "Node_JNI.h"
#include "VROSceneController.h"
#include "VRORenderer_JNI.h"
#include "VROReticle.h"
#include "SceneController_JNI.h"
#include "FrameListener_JNI.h"
#include "object.hpp"
#include "ARUtils_JNI.h"
#include "Camera_JNI.h"
#include "VRORenderer.h"
#include "VROChoreographer.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Renderer_##method_name

extern "C" {

// The renderer test runs VROSample.cpp, for fast prototyping when working
// on renderer features (no bridge integration). Do not check-in with this
// flag true!
static const bool kRunRendererTest = false;
static std::shared_ptr<VROSample> sample;

JNI_METHOD(jlong, nativeCreateRendererGVR)(JNIEnv *env, jclass clazz,
                                           jobject class_loader,
                                           jobject android_context,
                                           jobject asset_mgr,
                                           jobject platform_util,
                                           jlong native_gvr_api,
                                           jboolean enableShadows,
                                           jboolean enableHDR,
                                           jboolean enablePBR,
                                           jboolean enableBloom) {
    VROPlatformSetType(VROPlatformType::AndroidGVR);

    std::shared_ptr<gvr::AudioApi> gvrAudio = std::make_shared<gvr::AudioApi>();
    gvrAudio->Init(env, android_context, class_loader, GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    VROPlatformSetEnv(env, android_context, asset_mgr, platform_util);

    VRORendererConfiguration config;
    config.enableShadows = enableShadows;
    config.enableHDR = enableHDR;
    config.enablePBR = enablePBR;
    config.enableBloom = enableBloom;

    gvr_context *gvrContext = reinterpret_cast<gvr_context *>(native_gvr_api);
    std::shared_ptr<VROSceneRenderer> renderer
            = std::make_shared<VROSceneRendererGVR>(config, gvrContext, gvrAudio);
    return Renderer::jptr(renderer);
}

JNI_METHOD(jlong, nativeCreateRendererOVR)(JNIEnv *env, jclass clazz,
                                           jobject class_loader,
                                           jobject android_context,
                                           jobject view,
                                           jobject activity,
                                           jobject asset_mgr,
                                           jobject platform_util,
                                           jboolean enableShadows,
                                           jboolean enableHDR,
                                           jboolean enablePBR,
                                           jboolean enableBloom) {
    VROPlatformSetType(VROPlatformType::AndroidOVR);

    std::shared_ptr<gvr::AudioApi> gvrAudio = std::make_shared<gvr::AudioApi>();
    gvrAudio->Init(env, android_context, class_loader, GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    VROPlatformSetEnv(env, android_context, asset_mgr, platform_util);

    VRORendererConfiguration config;
    config.enableShadows = enableShadows;
    config.enableHDR = enableHDR;
    config.enablePBR = enablePBR;
    config.enableBloom = enableBloom;

    std::shared_ptr<VROSceneRenderer> renderer
            = std::make_shared<VROSceneRendererOVR>(config, gvrAudio, view, activity, env);
    return Renderer::jptr(renderer);
}

JNI_METHOD(jlong, nativeCreateRendererSceneView)(JNIEnv *env, jclass clazz,
                                                 jobject class_loader,
                                                 jobject android_context,
                                                 jobject view,
                                                 jobject asset_mgr,
                                                 jobject platform_util,
                                                 jboolean enableShadows,
                                                 jboolean enableHDR,
                                                 jboolean enablePBR,
                                                 jboolean enableBloom) {
    VROPlatformSetType(VROPlatformType::AndroidSceneView);

    std::shared_ptr<gvr::AudioApi> gvrAudio = std::make_shared<gvr::AudioApi>();
    gvrAudio->Init(env, android_context, class_loader, GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    VROPlatformSetEnv(env, android_context, asset_mgr, platform_util);

    VRORendererConfiguration config;
    config.enableShadows = enableShadows;
    config.enableHDR = enableHDR;
    config.enablePBR = enablePBR;
    config.enableBloom = enableBloom;

    std::shared_ptr<VROSceneRenderer> renderer
            = std::make_shared<VROSceneRendererSceneView>(config, gvrAudio, view);
    return Renderer::jptr(renderer);
}

JNI_METHOD(void, nativeDestroyRenderer)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_renderer) {
    Renderer::native(native_renderer)->onDestroy();
    VROThreadRestricted::unsetThread();

    delete reinterpret_cast<PersistentRef<VROSceneRenderer> *>(native_renderer);

    // Once the renderer dies, release/reset VROPlatformUtils stuff
    VROPlatformReleaseEnv();
}

JNI_METHOD(void, nativeInitializeGL)(JNIEnv *env,
                                     jobject obj,
                                     jlong native_renderer,
                                     jboolean sRGBFramebuffer) {

    VROThreadRestricted::setThread(VROThreadName::Renderer);
    std::shared_ptr<VROSceneRenderer> sceneRenderer = Renderer::native(native_renderer);

    std::shared_ptr<VRODriverOpenGLAndroid> driver = std::dynamic_pointer_cast<VRODriverOpenGLAndroid>(sceneRenderer->getDriver());
    driver->setSRGBFramebuffer(sRGBFramebuffer);

    if (kRunRendererTest) {
        sample = std::make_shared<VROSample>();
        sceneRenderer->setRenderDelegate(sample);

        sample->loadTestHarness(sceneRenderer->getRenderer(), sceneRenderer->getFrameSynchronizer(),
                                sceneRenderer->getDriver());
        sceneRenderer->setSceneController(sample->getSceneController());
        if (sample->getPointOfView()) {
            sceneRenderer->getRenderer()->setPointOfView(sample->getPointOfView());
        }
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

JNI_METHOD(void, nativeOnPinchEvent) (JNIEnv *env,
                                      jobject object,
                                      jlong native_renderer,
                                      jint pinchState,
                                      jfloat scaleFactor,
                                      jfloat viewportX,
                                      jfloat viewportY) {
    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(native_renderer);
    VROPlatformDispatchAsyncRenderer([renderer_w, pinchState, scaleFactor, viewportX, viewportY] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }
        renderer->onPinchEvent(pinchState, scaleFactor, viewportX, viewportY);
    });
}

JNI_METHOD(void, nativeOnRotateEvent) (JNIEnv *env,
                                       jobject object,
                                       jlong native_renderer,
                                       jint rotateState,
                                       jfloat rotateRadians,
                                       jfloat viewportX,
                                       jfloat viewportY) {
    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(native_renderer);
    VROPlatformDispatchAsyncRenderer([renderer_w, rotateState, rotateRadians, viewportX, viewportY] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }
        renderer->onRotateEvent(rotateState, rotateRadians, viewportX, viewportY);
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

JNI_METHOD(jfloatArray, nativeProjectPoint)(JNIEnv *env, jobject object, jlong renderer_j,
                                            jfloat x, jfloat y, jfloat z) {
    std::shared_ptr<VRORenderer> renderer = Renderer::native(renderer_j)->getRenderer();
    return ARUtilsCreateFloatArrayFromVector3f(renderer->projectPoint({ x, y, z }));
}

JNI_METHOD(jfloatArray, nativeUnprojectPoint)(JNIEnv *env, jobject object, jlong renderer_j,
                                              jfloat x, jfloat y, jfloat z) {
    std::shared_ptr<VRORenderer> renderer = Renderer::native(renderer_j)->getRenderer();
    return ARUtilsCreateFloatArrayFromVector3f(renderer->unprojectPoint({ x, y, z }));
}

JNI_METHOD(void, nativeSetClearColor)(JNIEnv *env,
                                           jobject object,
                                           jlong native_renderer,
                                           jint color) {
    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(native_renderer);
    VROPlatformDispatchAsyncRenderer([renderer_w, color] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }
        // Get the color
        float a = ((color >> 24) & 0xFF) / 255.0f;
        float r = ((color >> 16) & 0xFF) / 255.0f;
        float g = ((color >> 8) & 0xFF) / 255.0f;
        float b = (color & 0xFF) / 255.0f;

        VROVector4f vecColor(r, g, b, a);
        renderer->setClearColor(vecColor);
    });
}

JNI_METHOD(void, nativeAddFrameListener)(JNIEnv *env, jobject obj, jlong native_renderer, jlong frame_listener) {

    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(native_renderer);
    std::weak_ptr<VROFrameListener> frameListener_w  = FrameListener::native(frame_listener);

    VROPlatformDispatchAsyncRenderer([renderer_w, frameListener_w] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }

        std::shared_ptr<VROFrameListener> frameListener = frameListener_w.lock();
        if (!frameListener) {
            return;
        }
        renderer->getFrameSynchronizer()->addFrameListener(frameListener);
    });
}

JNI_METHOD(void, nativeRemoveFrameListener)(JNIEnv *env, jobject obj, jlong native_renderer, jlong frame_listener) {
    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(native_renderer);
    std::weak_ptr<VROFrameListener> frameListener_w  = FrameListener::native(frame_listener);

    VROPlatformDispatchAsyncRenderer([renderer_w, frameListener_w] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }

        std::shared_ptr<VROFrameListener> frameListener = frameListener_w.lock();
        if (!frameListener) {
            return;
        }

        renderer->getFrameSynchronizer()->removeFrameListener(frameListener);
    });
}

JNI_METHOD(jboolean, nativeIsReticlePointerFixed)(JNIEnv *env, jobject obj, jlong native_renderer) {
    std::shared_ptr<VROSceneRenderer> sceneRenderer = Renderer::native(native_renderer);
    return sceneRenderer->getRenderer()->getInputController()->getPresenter()->getReticle()->isHeadlocked();
}

JNI_METHOD(jfloatArray, nativeGetCameraPositionRealtime)(JNIEnv *env,
                                             jobject obj,
                                             jlong native_renderer) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    return ARUtilsCreateFloatArrayFromVector3f(renderer->getRenderer()->getCameraPositionRealTime());
}

JNI_METHOD(jfloatArray, nativeGetCameraRotationRealtime)(JNIEnv *env,
                                                         jobject obj,
                                                         jlong native_renderer) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    return ARUtilsCreateFloatArrayFromVector3f(renderer->getRenderer()->getCameraRotationRealTime());
}

JNI_METHOD(jfloatArray, nativeGetCameraForwardRealtime)(JNIEnv *env,
                                                         jobject obj,
                                                         jlong native_renderer) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    return ARUtilsCreateFloatArrayFromVector3f(renderer->getRenderer()->getCameraForwardRealTime());
}

JNI_METHOD(jfloat, nativeGetFieldOfView)(JNIEnv *env, jobject obj, jlong native_ref) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_ref);
    return renderer->getRenderer()->getActiveFieldOfView();
}

JNI_METHOD(void, nativeSetCameraListener)(JNIEnv *env,
                                            jobject obj,
                                            jlong native_renderer,
                                            jboolean enabled) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    if (enabled) {
        std::shared_ptr<CameraDelegateJNI> listener = std::make_shared<CameraDelegateJNI>(obj);
        renderer->getRenderer()->setCameraDelegate(listener);
    } else {
        renderer->getRenderer()->setCameraDelegate(nullptr);
    }
}

JNI_METHOD(void, nativeSetShadowsEnabled)(JNIEnv *env,
                                          jobject obj,
                                          jlong native_renderer,
                                          jboolean enabled) {
    std::weak_ptr<VROSceneRenderer> sceneRenderer_w = Renderer::native(native_renderer);

    VROPlatformDispatchAsyncRenderer([sceneRenderer_w, enabled] {
        std::shared_ptr<VROSceneRenderer> sceneRenderer = sceneRenderer_w.lock();
        if (!sceneRenderer) {
            return;
        }
        sceneRenderer->getRenderer()->getChoreographer()->setShadowsEnabled(enabled);
    });
}

JNI_METHOD(void, nativeSetHDREnabled)(JNIEnv *env,
                                      jobject obj,
                                      jlong native_renderer,
                                      jboolean enabled) {
    std::weak_ptr<VROSceneRenderer> sceneRenderer_w = Renderer::native(native_renderer);

    VROPlatformDispatchAsyncRenderer([sceneRenderer_w, enabled] {
        std::shared_ptr<VROSceneRenderer> sceneRenderer = sceneRenderer_w.lock();
        if (!sceneRenderer) {
            return;
        }
        sceneRenderer->getRenderer()->getChoreographer()->setHDREnabled(enabled);
    });
}

JNI_METHOD(void, nativeSetPBREnabled)(JNIEnv *env,
                                      jobject obj,
                                      jlong native_renderer,
                                      jboolean enabled) {
    std::weak_ptr<VROSceneRenderer> sceneRenderer_w = Renderer::native(native_renderer);

    VROPlatformDispatchAsyncRenderer([sceneRenderer_w, enabled] {
        std::shared_ptr<VROSceneRenderer> sceneRenderer = sceneRenderer_w.lock();
        if (!sceneRenderer) {
            return;
        }
        sceneRenderer->getRenderer()->getChoreographer()->setPBREnabled(enabled);
    });
}

JNI_METHOD(void, nativeSetBloomEnabled)(JNIEnv *env,
                                        jobject obj,
                                        jlong native_renderer,
                                        jboolean enabled) {
    std::weak_ptr<VROSceneRenderer> sceneRenderer_w = Renderer::native(native_renderer);

    VROPlatformDispatchAsyncRenderer([sceneRenderer_w, enabled] {
        std::shared_ptr<VROSceneRenderer> sceneRenderer = sceneRenderer_w.lock();
        if (!sceneRenderer) {
            return;
        }
        sceneRenderer->getRenderer()->getChoreographer()->setBloomEnabled(enabled);
    });
}

}  // extern "C"
