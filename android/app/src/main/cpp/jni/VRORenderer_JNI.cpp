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

#include <VROARHitTestResult.h>
#include <VROFrameListener.h>

#include "arcore/ARCore_JNI.h"

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "VROSceneRendererGVR.h"
#include "VROSceneRendererOVR.h"
#include "VROSceneRendererARCore.h"
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

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_Renderer_##method_name

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
                                              jni::Object<arcore::ViroViewARCore> view,
                                              jni::Object<arcore::Session> session,
                                              jobject asset_mgr,
                                              jobject platform_util) {
    std::shared_ptr<gvr::AudioApi> gvrAudio = std::make_shared<gvr::AudioApi>();
    gvrAudio->Init(env, android_context, class_loader, GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    VROPlatformSetEnv(env, android_context, asset_mgr, platform_util);

    std::shared_ptr<VROSceneRenderer> renderer
            = std::make_shared<VROSceneRendererARCore>(gvrAudio, session, view);
    return Renderer::jptr(renderer);
}


JNI_METHOD(jlong, nativeCreateRendererSceneView)(JNIEnv *env, jclass clazz,
                                                 jobject class_loader,
                                                 jobject android_context,
                                                 jobject view,
                                                 jobject asset_mgr,
                                                 jobject platform_util) {
    std::shared_ptr<gvr::AudioApi> gvrAudio = std::make_shared<gvr::AudioApi>();
    gvrAudio->Init(env, android_context, class_loader, GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    VROPlatformSetEnv(env, android_context, asset_mgr, platform_util);

    std::shared_ptr<VROSceneRenderer> renderer
            = std::make_shared<VROSceneRendererSceneView>(gvrAudio, view);
    return Renderer::jptr(renderer);
}

JNI_METHOD(void, nativeDestroyRenderer)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_renderer) {
    Renderer::native(native_renderer)->onDestroy();
    VROThreadRestricted::unsetThread(VROThreadName::Renderer);

    delete reinterpret_cast<PersistentRef<VROSceneRenderer> *>(native_renderer);

    // Once the renderer dies, release/reset VROPlatformUtils stuff
    VROPlatformReleaseEnv();
}

JNI_METHOD(void, nativeInitializeGl)(JNIEnv *env,
                                     jobject obj,
                                     jlong native_renderer) {

    VROThreadRestricted::setThread(VROThreadName::Renderer, pthread_self());
    std::shared_ptr<VROSceneRenderer> sceneRenderer = Renderer::native(native_renderer);

    if (kRunRendererTest) {
        sample = std::make_shared<VROSample>();
        sceneRenderer->setRenderDelegate(sample);

        sample->loadTestHarness(sceneRenderer->getFrameSynchronizer(), sceneRenderer->getDriver());
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

void invokeARResultsCallback(std::vector<VROARHitTestResult> &results, jweak weakCallback) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jclass arHitTestResultClass = env->FindClass("com/viro/renderer/jni/ARHitTestResult");

    jobjectArray resultsArray = env->NewObjectArray(results.size(), arHitTestResultClass, NULL);
    for (int i = 0; i < results.size(); i++) {
        VROARHitTestResult result = results[i];

        jmethodID constructorMethod = env->GetMethodID(arHitTestResultClass, "<init>", "(Ljava/lang/String;[F[F[F)V");
        jstring jtypeString;
        jfloatArray jposition = env->NewFloatArray(3);
        jfloatArray jscale = env->NewFloatArray(3);
        jfloatArray jrotation = env->NewFloatArray(3);

        VROVector3f positionVec = result.getWorldTransform().extractTranslation();
        VROVector3f scaleVec = result.getWorldTransform().extractScale();
        VROVector3f rotationVec = result.getWorldTransform().extractRotation(scaleVec).toEuler();

        float position[3] = {positionVec.x, positionVec.y, positionVec.z};
        float scale[3] = {scaleVec.x, scaleVec.y, scaleVec.z};
        float rotation[3] = {rotationVec.x, rotationVec.y, rotationVec.z};

        env->SetFloatArrayRegion(jposition, 0, 3, position);
        env->SetFloatArrayRegion(jscale, 0, 3, scale);
        env->SetFloatArrayRegion(jrotation, 0, 3, rotation);

        const char* typeString;
        // Note: ARCore currently only supports Plane & FeaturePoint. See VROARFrameARCore::hitTest.
        switch (result.getType()) {
            case VROARHitTestResultType::ExistingPlaneUsingExtent:
                typeString = "ExistingPlaneUsingExtent";
                break;
            default: // FeaturePoint
                typeString = "FeaturePoint";
                break;
        }

        jtypeString = env->NewStringUTF(typeString);
        jobject jresult = env->NewObject(arHitTestResultClass, constructorMethod, jtypeString,
                                         jposition, jscale, jrotation);
        env->SetObjectArrayElement(resultsArray, i, jresult);
    }

    jobject globalArrayRef = env->NewGlobalRef(resultsArray);
    VROPlatformDispatchAsyncApplication([weakCallback, globalArrayRef] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject callback = env->NewLocalRef(weakCallback);
        VROPlatformCallJavaFunction(callback, "onHitTestFinished",
                                    "([Lcom/viro/renderer/jni/ARHitTestResult;)V",
                                    globalArrayRef);
        env->DeleteGlobalRef(globalArrayRef);
    });
}

void invokeEmptyARResultsCallback(jweak weakCallback) {
    VROPlatformDispatchAsyncApplication([weakCallback] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject callback = env->NewLocalRef(weakCallback);
        jclass arHitTestResultClass = env->FindClass("com/viro/renderer/jni/ARHitTestResult");
        jobjectArray emptyArray = env->NewObjectArray(0, arHitTestResultClass, NULL);
        VROPlatformCallJavaFunction(callback, "onHitTestFinished",
                                    "([Lcom/viro/renderer/jni/ARHitTestResult;)V", emptyArray);
    });
}

void performARHitTest(VROVector3f rayVec, std::weak_ptr<VROSceneRendererARCore> arRenderer_w,
                      jweak weakCallback) {
    std::shared_ptr<VROSceneRendererARCore> arRenderer = arRenderer_w.lock();
    if (!arRenderer) {
        invokeEmptyARResultsCallback(weakCallback);
    }
    else {
        std::vector<VROARHitTestResult> results = arRenderer->performARHitTest(rayVec);
        invokeARResultsCallback(results, weakCallback);
    }
}

void performARHitTestPoint(JNIEnv *env, float x, float y, std::weak_ptr<VROSceneRendererARCore> arRenderer_w,
                           jweak weakCallback) {
    std::shared_ptr<VROSceneRendererARCore> arRenderer = arRenderer_w.lock();
    if (!arRenderer) {
        invokeEmptyARResultsCallback(weakCallback);
    }
    else {
        std::vector<VROARHitTestResult> results = arRenderer->performARHitTest(x, y);
        invokeARResultsCallback(results, weakCallback);
    }
}

JNI_METHOD(void, nativePerformARHitTestWithRay) (JNIEnv *env,
                                                 jobject object,
                                                 jlong native_renderer,
                                                 jfloatArray ray,
                                                 jobject callback) {
    // Grab ray to perform the AR hit test
    jfloat *rayStart = env->GetFloatArrayElements(ray, 0);
    VROVector3f rayVec = VROVector3f(rayStart[0], rayStart[1], rayStart[2]);
    env->ReleaseFloatArrayElements(ray, rayStart, 0);

    // Create weak pointers for dispatching
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    std::weak_ptr<VROSceneRendererARCore> arRenderer_w = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);
    jweak weakCallback = env->NewWeakGlobalRef(callback);

    VROPlatformDispatchAsyncRenderer([arRenderer_w, weakCallback, rayVec] {
        performARHitTest(rayVec, arRenderer_w, weakCallback);
    });
}

JNI_METHOD(void, nativePerformARHitTestWithPosition) (JNIEnv *env,
                                                      jobject object,
                                                      jlong native_renderer,
                                                      jfloatArray position,
                                                      jobject callback) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);

    // Calculate ray to perform the AR hit test
    jfloat *positionStart = env->GetFloatArrayElements(position, 0);
    VROVector3f positionVec = VROVector3f(positionStart[0], positionStart[1], positionStart[2]);
    env->ReleaseFloatArrayElements(position, positionStart, 0);

    VROVector3f cameraVec = renderer->getRenderer()->getRenderContext()->getCamera().getPosition();
    // the ray we want to use is (given position - camera position)
    VROVector3f rayVec = positionVec - cameraVec;

    // Create weak pointers for dispatching
    std::weak_ptr<VROSceneRendererARCore> arRenderer_w = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);
    jweak weakCallback = env->NewWeakGlobalRef(callback);

    VROPlatformDispatchAsyncRenderer([arRenderer_w, weakCallback, rayVec] {
        performARHitTest(rayVec, arRenderer_w, weakCallback);
    });
}

JNI_METHOD(void, nativePerformARHitTestWithPoint) (JNIEnv *env,
                                                   jobject object,
                                                   jlong native_renderer,
                                                   jfloat x, jfloat y,
                                                   jobject callback) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    std::weak_ptr<VROSceneRendererARCore> arRenderer_w = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);
    jweak weakCallback = env->NewWeakGlobalRef(callback);

    VROPlatformDispatchAsyncRenderer([env, arRenderer_w, weakCallback, x, y] {
        performARHitTestPoint(env, x, y, arRenderer_w, weakCallback);
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
    return sceneRenderer->getRenderer()->getInputController()->getPresenter()->getReticle()->isPointerFixed();
}

}  // extern "C"
