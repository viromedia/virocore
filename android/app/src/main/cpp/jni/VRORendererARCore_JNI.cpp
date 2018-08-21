//
//  VRORendererARCore_JNI.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 2/24/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include "VRORenderer_JNI.h"
#include <PersistentRef.h>
#include "VROSceneRendererARCore.h"
#include "ViroUtils_JNI.h"
#include "VRORenderer.h"
#include "ViroContextAndroid_JNI.h"
#include "VROCameraImageListener.h"
#include "arcore/ARUtils_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_RendererARCore_##method_name
#endif

extern "C" {

VRO_METHOD(jlong, nativeCreateRendererARCore)(VRO_ARGS
                                              jobject class_loader,
                                              jobject android_context,
                                              jobject asset_mgr,
                                              jobject platform_util,
                                              jboolean enableShadows,
                                              jboolean enableHDR,
                                              jboolean enablePBR,
                                              jboolean enableBloom) {
    VROPlatformSetType(VROPlatformType::AndroidARCore);

    std::shared_ptr<gvr::AudioApi> gvrAudio = std::make_shared<gvr::AudioApi>();
    gvrAudio->Init(env, android_context, class_loader, GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    VROPlatformSetEnv(env, android_context, asset_mgr, platform_util);

    VRORendererConfiguration config;
    config.enableShadows = enableShadows;
    config.enableHDR = enableHDR;
    config.enablePBR = enablePBR;
    config.enableBloom = enableBloom;

    std::shared_ptr<VROSceneRenderer> renderer
            = std::make_shared<VROSceneRendererARCore>(config, gvrAudio);
    return Renderer::jptr(renderer);
}

VRO_METHOD(VRO_INT, nativeGetCameraTextureId)(VRO_ARGS
                                              jlong renderer_j) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(renderer_j);
    std::shared_ptr<VROSceneRendererARCore> arRenderer = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);
    return arRenderer->getCameraTextureId();
}

VRO_METHOD(void, nativeSetARCoreSession)(VRO_ARGS
                                         jlong renderer_j,
                                         jlong session_j) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(renderer_j);
    std::shared_ptr<VROSceneRendererARCore> arRenderer = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);
    arRenderer->setARCoreSession(reinterpret_cast<arcore::Session *>(session_j));
}

VRO_METHOD(void, nativeSetARDisplayGeometry)(VRO_ARGS
                                             jlong renderer_j,
                                             jint rotation, jint width, jint height) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(renderer_j);
    std::shared_ptr<VROSceneRendererARCore> arRenderer = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);
    arRenderer->setDisplayGeometry(rotation, width, height);
}

VRO_METHOD(void, nativeSetAnchorDetectionTypes)(VRO_ARGS
                                                jlong renderer_j,
                                                VRO_STRING_ARRAY typeStrArray) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(renderer_j);
    std::shared_ptr<VROSceneRendererARCore> arRenderer = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);

    std::set<VROAnchorDetection> types;

    int stringCount = VRO_ARRAY_LENGTH(typeStrArray);
    for (int i = 0; i < stringCount; i++) {
        std::string typeString = VRO_STRING_STL(VRO_STRING_ARRAY_GET(typeStrArray, i));

        if (VROStringUtil::strcmpinsensitive(typeString, "PlanesHorizontal")) {
            types.insert(VROAnchorDetection::PlanesHorizontal);
        } else if (VROStringUtil::strcmpinsensitive(typeString, "PlanesVertical")) {
            types.insert(VROAnchorDetection::PlanesVertical);
        }
    }

    arRenderer->setAnchorDetectionTypes(types);

}

void invokeARResultsCallback(std::vector<std::shared_ptr<VROARHitTestResult>> &results, jweak weakCallback) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jclass arHitTestResultClass = env->FindClass("com/viro/core/ARHitTestResult");

    jobjectArray resultsArray = env->NewObjectArray(results.size(), arHitTestResultClass, NULL);
    for (int i = 0; i < results.size(); i++) {
        jobject result = ARUtilsCreateARHitTestResult(results[i]);
        env->SetObjectArrayElement(resultsArray, i, result);
    }

    jobject globalArrayRef = env->NewGlobalRef(resultsArray);
    VROPlatformDispatchAsyncApplication([weakCallback, globalArrayRef] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject callback = env->NewLocalRef(weakCallback);
        VROPlatformCallHostFunction(callback, "onHitTestFinished",
                                    "([Lcom/viro/core/ARHitTestResult;)V",
                                    globalArrayRef);
        env->DeleteGlobalRef(globalArrayRef);
        env->DeleteWeakGlobalRef(weakCallback);
    });
}

void invokeEmptyARResultsCallback(jweak weakCallback) {
    VROPlatformDispatchAsyncApplication([weakCallback] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject callback = env->NewLocalRef(weakCallback);
        jclass arHitTestResultClass = env->FindClass("com/viro/core/ARHitTestResult");
        jobjectArray emptyArray = env->NewObjectArray(0, arHitTestResultClass, NULL);
        VROPlatformCallHostFunction(callback, "onHitTestFinished",
                                    "([Lcom/viro/core/ARHitTestResult;)V", emptyArray);
        env->DeleteWeakGlobalRef(weakCallback);
    });
}

void performARHitTest(VROVector3f rayVec, std::weak_ptr<VROSceneRendererARCore> arRenderer_w,
                      jweak weakCallback) {
    std::shared_ptr<VROSceneRendererARCore> arRenderer = arRenderer_w.lock();
    if (!arRenderer) {
        invokeEmptyARResultsCallback(weakCallback);
    }
    else {
        std::vector<std::shared_ptr<VROARHitTestResult>> results = arRenderer->performARHitTest(rayVec);
        invokeARResultsCallback(results, weakCallback);
    }
}

void performARHitTestRay(VROVector3f rayOrigin, VROVector3f rayDestination, std::weak_ptr<VROSceneRendererARCore> arRenderer_w,
                      jweak weakCallback) {
    std::shared_ptr<VROSceneRendererARCore> arRenderer = arRenderer_w.lock();
    if (!arRenderer) {
        invokeEmptyARResultsCallback(weakCallback);
    }
    else {
        std::vector<std::shared_ptr<VROARHitTestResult>> results = arRenderer->performARHitTest(rayOrigin, rayDestination);
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
        std::vector<std::shared_ptr<VROARHitTestResult>> results = arRenderer->performARHitTest(x, y);
        invokeARResultsCallback(results, weakCallback);
    }
}

VRO_METHOD(void, nativePerformARHitTestWithRay) (VRO_ARGS
                                                 jlong native_renderer,
                                                 jfloatArray ray,
                                                 jobject callback) {
    // Grab ray to perform the AR hit test
    VRO_FLOAT *rayStart = VRO_FLOAT_ARRAY_GET_ELEMENTS(ray);
    VROVector3f rayVec = VROVector3f(rayStart[0], rayStart[1], rayStart[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(ray, rayStart);

    // Create weak pointers for dispatching
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    std::weak_ptr<VROSceneRendererARCore> arRenderer_w = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);
    jweak weakCallback = env->NewWeakGlobalRef(callback);

    VROPlatformDispatchAsyncRenderer([arRenderer_w, weakCallback, rayVec] {
        performARHitTest(rayVec, arRenderer_w, weakCallback);
    });
}

VRO_METHOD(void, nativePerformARHitTestWithOriginDestRay) (VRO_ARGS
                                                 jlong native_renderer,
                                                 jfloatArray origin,
                                                 jfloatArray destination,
                                                 jobject callback) {
    // Grab ray origin to perform the AR hit test
    VRO_FLOAT *rayStart = VRO_FLOAT_ARRAY_GET_ELEMENTS(origin);
    VROVector3f rayOrigVec = VROVector3f(rayStart[0], rayStart[1], rayStart[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(origin, rayStart);

    // Grab ray destination to perform the AR hit test
    VRO_FLOAT *rayEnd = VRO_FLOAT_ARRAY_GET_ELEMENTS(destination);
    VROVector3f rayDestVec = VROVector3f(rayEnd[0], rayEnd[1], rayEnd[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(destination, rayEnd);

    // Create weak pointers for dispatching
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);
    std::weak_ptr<VROSceneRendererARCore> arRenderer_w = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);
    jweak weakCallback = env->NewWeakGlobalRef(callback);

    VROPlatformDispatchAsyncRenderer([arRenderer_w, weakCallback, rayOrigVec, rayDestVec] {
        performARHitTestRay(rayOrigVec, rayDestVec, arRenderer_w, weakCallback);
    });
}

VRO_METHOD(void, nativePerformARHitTestWithPosition) (VRO_ARGS
                                                      jlong native_renderer,
                                                      jfloatArray position,
                                                      jobject callback) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(native_renderer);

    // Calculate ray to perform the AR hit test
    VRO_FLOAT *positionStart = VRO_FLOAT_ARRAY_GET_ELEMENTS(position);
    VROVector3f positionVec = VROVector3f(positionStart[0], positionStart[1], positionStart[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(position, positionStart);

    VROVector3f cameraVec = renderer->getRenderer()->getCamera().getPosition();
    // the ray we want to use is (given position - camera position)
    VROVector3f rayVec = positionVec - cameraVec;

    // Create weak pointers for dispatching
    std::weak_ptr<VROSceneRendererARCore> arRenderer_w = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);
    jweak weakCallback = env->NewWeakGlobalRef(callback);

    VROPlatformDispatchAsyncRenderer([arRenderer_w, weakCallback, rayVec] {
        performARHitTest(rayVec, arRenderer_w, weakCallback);
    });
}

VRO_METHOD(void, nativePerformARHitTestWithPoint) (VRO_ARGS
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

VRO_METHOD(void, nativeEnableTracking) (VRO_ARGS
                                        jlong nativeRenderer,
                                        jboolean shouldTrack) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(nativeRenderer);
    std::weak_ptr<VROSceneRendererARCore> arRenderer_w = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);

    VROPlatformDispatchAsyncRenderer([arRenderer_w, shouldTrack]{
        std::shared_ptr<VROSceneRendererARCore> arRenderer = arRenderer_w.lock();
        if (arRenderer) {
            arRenderer->enableTracking(shouldTrack);
        }
    });
}

VRO_METHOD(void, nativeSetCameraImageListener)(VRO_ARGS
                                               VRO_REF(VROARSceneController) renderer_j,
                                               VRO_REF(ViroContext) context_j,
                                               VRO_OBJECT listener_j) {
    VRO_METHOD_PREAMBLE;
    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(renderer_j);
    std::weak_ptr<ViroContext> context_w = VRO_REF_GET(ViroContext, context_j);

    if (VRO_IS_OBJECT_NULL(listener_j)) {
        VROPlatformDispatchAsyncRenderer([context_w] {
            VRO_ENV env = VROPlatformGetJNIEnv();
            std::shared_ptr<ViroContext> context = context_w.lock();
            if (!context) {
                return;
            }
            std::shared_ptr<ViroContextAndroid> context_a = std::dynamic_pointer_cast<ViroContextAndroid>(context);
            if (!context_a) {
                return;
            }
            context_a->setCameraImageFrameListener(nullptr);
        });
    }
    else {
        VRO_OBJECT listener_g = VRO_NEW_GLOBAL_REF(listener_j);
        VROPlatformDispatchAsyncRenderer([listener_g, renderer_w, context_w] {
            VRO_ENV env = VROPlatformGetJNIEnv();
            std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
            if (!renderer) {
                VRO_DELETE_GLOBAL_REF(listener_g);
                return;
            }
            std::shared_ptr<VROSceneRendererARCore> renderer_arc = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);
            if (!renderer_arc) {
                VRO_DELETE_GLOBAL_REF(listener_g);
                return;
            }
            std::shared_ptr<ViroContext> context = context_w.lock();
            if (!context) {
                VRO_DELETE_GLOBAL_REF(listener_g);
                return;
            }
            std::shared_ptr<ViroContextAndroid> context_a = std::dynamic_pointer_cast<ViroContextAndroid>(context);
            if (!context_a) {
                VRO_DELETE_GLOBAL_REF(listener_g);
                return;
            }

            std::shared_ptr<VROCameraImageFrameListener> imageListener =
                    std::make_shared<VROCameraImageFrameListener>(listener_g, renderer_arc, env);
            context_a->setCameraImageFrameListener(imageListener);
            VRO_DELETE_GLOBAL_REF(listener_g);
        });
    }
}

VRO_METHOD(void, nativeSetCameraAutoFocusEnabled) (VRO_ARGS
                                        jlong nativeRenderer,
                                        jboolean enabled) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(nativeRenderer);
    std::weak_ptr<VROSceneRendererARCore> arRenderer_w = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);

    VROPlatformDispatchAsyncRenderer([arRenderer_w, enabled]{
        std::shared_ptr<VROSceneRendererARCore> arRenderer = arRenderer_w.lock();
        if (arRenderer) {
            arRenderer->setCameraAutoFocusEnabled(enabled);
        }
    });
}

VRO_METHOD(VRO_BOOL, nativeisCameraAutoFocusEnabled) (VRO_ARGS
                                                   jlong nativeRenderer) {
    std::shared_ptr<VROSceneRenderer> renderer = Renderer::native(nativeRenderer);
    std::shared_ptr<VROSceneRendererARCore> arRenderer = std::dynamic_pointer_cast<VROSceneRendererARCore>(renderer);
    return arRenderer->isCameraAutoFocusEnabled();
}


}



