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
#include "ARUtils_JNI.h"
#include "Camera_JNI.h"
#include "VRORenderer.h"
#include "VROChoreographer.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Renderer_##method_name
#endif

extern "C" {

// The renderer test runs VROSample.cpp, for fast prototyping when working
// on renderer features (no bridge integration).
static bool kRunRendererTest = false;
static std::shared_ptr<VROSample> sample;

VRO_METHOD(VRO_REF, nativeCreateRendererGVR)(VRO_ARGS
                                             jobject class_loader,
                                             jobject android_context,
                                             jobject asset_mgr,
                                             jobject platform_util,
                                             VRO_REF native_gvr_api,
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

VRO_METHOD(VRO_REF, nativeCreateRendererOVR)(VRO_ARGS
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

VRO_METHOD(VRO_REF, nativeCreateRendererSceneView)(VRO_ARGS
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

VRO_METHOD(void, nativeDestroyRenderer)(VRO_ARGS
                                        VRO_REF native_renderer) {
    Renderer::native(native_renderer)->onDestroy();
    VROThreadRestricted::unsetThread();

    delete reinterpret_cast<PersistentRef<VROSceneRenderer> *>(native_renderer);

    // Once the renderer dies, release/reset VROPlatformUtils stuff
    VROPlatformReleaseEnv();
}

VRO_METHOD(void, nativeInitializeGL)(VRO_ARGS
                                     VRO_REF native_renderer,
                                     jboolean sRGBFramebuffer,
                                     jboolean testingMode) {

    VROThreadRestricted::setThread(VROThreadName::Renderer);
    std::shared_ptr<VROSceneRenderer> sceneRenderer = Renderer::native(native_renderer);

    std::shared_ptr<VRODriverOpenGLAndroid> driver = std::dynamic_pointer_cast<VRODriverOpenGLAndroid>(sceneRenderer->getDriver());
    driver->setSRGBFramebuffer(sRGBFramebuffer);

    kRunRendererTest = testingMode;
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

VRO_METHOD(void, nativeDrawFrame)(VRO_ARGS
                                  VRO_REF native_renderer) {
    Renderer::native(native_renderer)->onDrawFrame();
}

VRO_METHOD (void, nativeOnKeyEvent)(VRO_ARGS
                                    VRO_REF native_renderer,
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

VRO_METHOD(void, nativeOnTouchEvent)(VRO_ARGS
                                     VRO_REF native_renderer,
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

VRO_METHOD(void, nativeOnPinchEvent) (VRO_ARGS
                                      VRO_REF native_renderer,
                                      jint pinchState,
                                      VRO_FLOAT scaleFactor,
                                      VRO_FLOAT viewportX,
                                      VRO_FLOAT viewportY) {
    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(native_renderer);
    VROPlatformDispatchAsyncRenderer([renderer_w, pinchState, scaleFactor, viewportX, viewportY] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }
        renderer->onPinchEvent(pinchState, scaleFactor, viewportX, viewportY);
    });
}

VRO_METHOD(void, nativeOnRotateEvent) (VRO_ARGS
                                       VRO_REF native_renderer,
                                       jint rotateState,
                                       VRO_FLOAT rotateRadians,
                                       VRO_FLOAT viewportX,
                                       VRO_FLOAT viewportY) {
    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(native_renderer);
    VROPlatformDispatchAsyncRenderer([renderer_w, rotateState, rotateRadians, viewportX, viewportY] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }
        renderer->onRotateEvent(rotateState, rotateRadians, viewportX, viewportY);
    });
}

VRO_METHOD(void, nativeSetVRModeEnabled)(VRO_ARGS
                                         VRO_REF nativeRenderer, jboolean enabled) {
    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(nativeRenderer);
    VROPlatformDispatchAsyncRenderer([renderer_w, enabled] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (!renderer) {
            return;
        }
        renderer->setVRModeEnabled(enabled);
    });
}

VRO_METHOD(void, nativeOnStart)(VRO_ARGS
                                VRO_REF native_renderer) {
        Renderer::native(native_renderer)->onStart();
}

VRO_METHOD(void, nativeOnPause)(VRO_ARGS
                                VRO_REF native_renderer) {
        Renderer::native(native_renderer)->onPause();
}

VRO_METHOD(void, nativeOnResume)(VRO_ARGS
                                 VRO_REF native_renderer) {
        Renderer::native(native_renderer)->onResume();
}

VRO_METHOD(void, nativeOnStop)(VRO_ARGS
                               VRO_REF native_renderer) {
        Renderer::native(native_renderer)->onStop();
}

VRO_METHOD(void, nativeSetSceneController)(VRO_ARGS
                                           VRO_REF native_renderer,
                                           VRO_REF native_scene_controller_ref) {
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

VRO_METHOD(void, nativeSetSceneControllerWithAnimation)(VRO_ARGS
                                                        VRO_REF native_renderer,
                                                        VRO_REF native_scene_controller_ref,
                                                        VRO_FLOAT duration) {
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

VRO_METHOD(void, nativeSetPointOfView)(VRO_ARGS
                                       VRO_REF native_renderer,
                                       VRO_REF native_node_ref) {
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

VRO_METHOD(void, nativeOnSurfaceCreated)(VRO_ARGS
                                         jobject surface,
                                         VRO_REF native_renderer) {

    Renderer::native(native_renderer)->onSurfaceCreated(surface);
}

VRO_METHOD(void, nativeOnSurfaceChanged)(VRO_ARGS
                                         jobject surface,
                                         jint width,
                                         jint height,
                                         VRO_REF native_renderer) {
    Renderer::native(native_renderer)->onSurfaceChanged(surface, width, height);
}

VRO_METHOD(void, nativeOnSurfaceDestroyed)(VRO_ARGS
                                           VRO_REF native_renderer) {
    Renderer::native(native_renderer)->onSurfaceDestroyed();
}

VRO_METHOD(jstring, nativeGetHeadset)(VRO_ARGS
                                      VRO_REF nativeRenderer) {
    std::string headset = Renderer::native(nativeRenderer)->getRenderer()->getInputController()->getHeadset();
    return env->NewStringUTF(headset.c_str());
}

VRO_METHOD(jstring, nativeGetController)(VRO_ARGS
                                         VRO_REF nativeRenderer) {
    std::string controller = Renderer::native(nativeRenderer)->getRenderer()->getInputController()->getController();
    return env->NewStringUTF(controller.c_str());
}

VRO_METHOD(void, nativeSetDebugHUDEnabled)(VRO_ARGS
                                           VRO_REF native_renderer,
                                           jboolean enabled) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    renderer->getRenderer()->setDebugHUDEnabled(enabled);
}

VRO_METHOD(void, nativeSetSuspended)(VRO_ARGS
                                     VRO_REF native_renderer,
                                     jboolean suspend_renderer) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    renderer->setSuspended(suspend_renderer);
}

// This function is OVR only!
VRO_METHOD(void, nativeRecenterTracking)(VRO_ARGS
                                         VRO_REF native_renderer) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    std::shared_ptr<VROSceneRendererOVR> ovrRenderer = std::dynamic_pointer_cast<VROSceneRendererOVR>(renderer);
    ovrRenderer->recenterTracking();
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeProjectPoint)(VRO_ARGS
                                                VRO_REF renderer_j,
                                                VRO_FLOAT x, VRO_FLOAT y, VRO_FLOAT z) {
    std::shared_ptr<VRORenderer> renderer = Renderer::native(renderer_j)->getRenderer();
    return ARUtilsCreateFloatArrayFromVector3f(renderer->projectPoint({ x, y, z }));
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeUnprojectPoint)(VRO_ARGS
                                                  VRO_REF renderer_j,
                                                  VRO_FLOAT x, VRO_FLOAT y, VRO_FLOAT z) {
    std::shared_ptr<VRORenderer> renderer = Renderer::native(renderer_j)->getRenderer();
    return ARUtilsCreateFloatArrayFromVector3f(renderer->unprojectPoint({ x, y, z }));
}

VRO_METHOD(void, nativeSetClearColor)(VRO_ARGS
                                      VRO_REF native_renderer,
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

VRO_METHOD(void, nativeAddFrameListener)(VRO_ARGS
                                         VRO_REF native_renderer, VRO_REF frame_listener) {

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

VRO_METHOD(void, nativeRemoveFrameListener)(VRO_ARGS
                                            VRO_REF native_renderer, VRO_REF frame_listener) {
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

VRO_METHOD(jboolean, nativeIsReticlePointerFixed)(VRO_ARGS
                                                  VRO_REF native_renderer) {
    std::shared_ptr<VROSceneRenderer> sceneRenderer = Renderer::native(native_renderer);
    return sceneRenderer->getRenderer()->getInputController()->getPresenter()->getReticle()->isHeadlocked();
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetCameraPositionRealtime)(VRO_ARGS
                                                             VRO_REF native_renderer) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    return ARUtilsCreateFloatArrayFromVector3f(renderer->getRenderer()->getCameraPositionRealTime());
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetCameraRotationRealtime)(VRO_ARGS
                                                             VRO_REF native_renderer) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    return ARUtilsCreateFloatArrayFromVector3f(renderer->getRenderer()->getCameraRotationRealTime());
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetCameraForwardRealtime)(VRO_ARGS
                                                            VRO_REF native_renderer) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    return ARUtilsCreateFloatArrayFromVector3f(renderer->getRenderer()->getCameraForwardRealTime());
}

VRO_METHOD(VRO_FLOAT, nativeGetFieldOfView)(VRO_ARGS
                                            VRO_REF native_ref) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_ref);
    return renderer->getRenderer()->getActiveFieldOfView();
}

VRO_METHOD(void, nativeSetCameraListener)(VRO_ARGS
                                          VRO_REF native_renderer,
                                          jboolean enabled) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    if (enabled) {
        std::shared_ptr<CameraDelegateJNI> listener = std::make_shared<CameraDelegateJNI>(obj);
        renderer->getRenderer()->setCameraDelegate(listener);
    } else {
        renderer->getRenderer()->setCameraDelegate(nullptr);
    }
}

VRO_METHOD(void, nativeSetShadowsEnabled)(VRO_ARGS
                                          VRO_REF native_renderer,
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

VRO_METHOD(void, nativeSetHDREnabled)(VRO_ARGS
                                      VRO_REF native_renderer,
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

VRO_METHOD(void, nativeSetPBREnabled)(VRO_ARGS
                                      VRO_REF native_renderer,
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

VRO_METHOD(void, nativeSetBloomEnabled)(VRO_ARGS
                                        VRO_REF native_renderer,
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
