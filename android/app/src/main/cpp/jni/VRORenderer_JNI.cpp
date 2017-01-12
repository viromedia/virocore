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

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "VROSceneRendererGVR.h"
#include "VROSceneRendererOVR.h"
#include "VROPlatformUtil.h"
#include "VROSample.h"
#include "VROSceneController.h"
#include "VRORenderer_JNI.h"
#include "VROReticle.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_RendererJni_##method_name

extern "C" {

static std::shared_ptr<VROSample> sample;

JNI_METHOD(jlong, nativeCreateRendererGVR)(JNIEnv *env, jclass clazz,
                                           jobject class_loader,
                                           jobject android_context,
                                           jobject asset_mgr,
                                           jobject platform_util,
                                           jlong native_gvr_api) {
    std::shared_ptr<gvr::AudioApi> gvrAudio = std::make_shared<gvr::AudioApi>();
    gvrAudio->Init(env, android_context, class_loader, GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    VROPlatformSetEnv(env, asset_mgr, platform_util);

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
    VROPlatformSetEnv(env, asset_mgr, platform_util);

    std::shared_ptr<VROSceneRenderer> renderer
            = std::make_shared<VROSceneRendererOVR>(gvrAudio, view, activity, env);
    return Renderer::jptr(renderer);
}

JNI_METHOD(void, nativeDestroyRenderer)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_renderer) {
    VROPlatformReleaseEnv();
    delete reinterpret_cast<PersistentRef<VROSceneRenderer> *>(native_renderer);
}

JNI_METHOD(void, nativeInitializeGl)(JNIEnv *env,
                                     jobject obj,
                                     jlong native_renderer) {
    // TODO Temporary place for sample
    // sample = std::make_shared<VROSample>();
    // sceneRenderer->setRenderDelegate(sample);
    // sceneRenderer->setSceneController(sample->loadBoxScene(sceneRenderer->getFrameSynchronizer(),
    //                                                       sceneRenderer->getDriver()));
    Renderer::native(native_renderer)->initGL();
}

JNI_METHOD(void, nativeDrawFrame)(JNIEnv *env,
                                  jobject obj,
                                  jlong native_renderer) {
    Renderer::native(native_renderer)->onDrawFrame();
}

JNI_METHOD(void, nativeOnTriggerEvent)(JNIEnv *env,
                                       jobject obj,
                                       jlong native_renderer) {
    Renderer::native(native_renderer)->onTriggerEvent();
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

JNI_METHOD(void, nativeSetScene)(JNIEnv *env,
                                 jobject obj,
                                 jlong native_renderer,
                                 jlong native_scene_controller_ref) {
    VROSceneController *scene_controller = reinterpret_cast<VROSceneController *>(native_scene_controller_ref);
    std::shared_ptr<VROSceneController> shared_controller = std::shared_ptr<VROSceneController>(scene_controller);
    Renderer::native(native_renderer)->setSceneController(shared_controller);
}

JNI_METHOD(void, nativeEnableReticle)(JNIEnv *env,
                                 jobject obj,
                                 jlong native_renderer,
                                 jboolean enable) {
    Renderer::native(native_renderer)->getRenderer()->getReticle()->setEnabled(enable);
}

JNI_METHOD(void, nativeSetCameraPosition)(JNIEnv *env,
                                          jobject obj,
                                          jlong nativeRenderer, jfloat x, jfloat y, jfloat z) {
    Renderer::native(nativeRenderer)->getRenderer()->setPosition(VROVector3f(x, y, z));
}

JNI_METHOD(void, nativeSetCameraRotationType)(JNIEnv *env,
                                              jobject obj,
                                              jlong nativeRenderer,
                                              jstring rotationType) {
    // Get the string
    const char *cStrRotationType = env->GetStringUTFChars(rotationType, NULL);
    std::string strRotationType(cStrRotationType);

    if (VROStringUtil::strcmpinsensitive(strRotationType, "orbit")) {
        Renderer::native(nativeRenderer)->getRenderer()->setCameraRotationType(VROCameraRotationType::Orbit);
    } else {
        // default rotation type is standard.
        Renderer::native(nativeRenderer)->getRenderer()->setCameraRotationType(VROCameraRotationType::Standard);
    }
    env->ReleaseStringUTFChars(rotationType, cStrRotationType);
}

JNI_METHOD(void, nativeSetOrbitCameraFocalPoint)(JNIEnv *env,
                                                 jobject obj,
                                                 jlong nativeRenderer, jfloat x, jfloat y, jfloat z) {
    Renderer::native(nativeRenderer)->getRenderer()->setOrbitFocalPoint(VROVector3f(x, y, z));
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
                                jlong native_renderer) {
    Renderer::native(native_renderer)->onSurfaceChanged(surface);
}

JNI_METHOD(void, nativeOnSurfaceDestroyed)(JNIEnv *env,
                                jobject obj,
                                jlong native_renderer) {
    Renderer::native(native_renderer)->onSurfaceDestroyed();
}

}  // extern "C"
