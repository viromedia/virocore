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

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "VROSceneRendererCardboard.h"
#include "VROPlatformUtil.h"
#include "VROSample.h"
#include "VROSceneController.h"
#include "VRORenderer_JNI.h"
#include "RenderContext_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_ViroGvrLayout_##method_name

extern "C" {

static std::shared_ptr<VROSample> sample;

JNI_METHOD(jlong, nativeCreateRenderer)(JNIEnv *env, jclass clazz,
                                        jobject class_loader,
                                        jobject gvr_layout,
                                        jobject android_context,
                                        jobject asset_mgr,
                                        jlong native_gvr_api) {
    std::shared_ptr<gvr::AudioApi> gvrAudio = std::make_shared<gvr::AudioApi>();
    gvrAudio->Init(env, android_context, class_loader, GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    VROPlatformSetEnv(env, gvr_layout, asset_mgr);

    gvr_context *gvrContext = reinterpret_cast<gvr_context *>(native_gvr_api);
    std::shared_ptr<VROSceneRendererCardboard> renderer
            = std::make_shared<VROSceneRendererCardboard>(gvrContext, gvrAudio);
    return Renderer::jptr(renderer);
}

JNI_METHOD(void, nativeDestroyRenderer)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_renderer) {
    VROPlatformReleaseEnv();
    delete reinterpret_cast<PersistentRef<VROSceneRendererCardboard> *>(native_renderer);
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

JNI_METHOD(void, nativeSetScene)(JNIEnv *env,
                                 jobject obj,
                                 jlong native_renderer,
                                 jlong native_scene_controller_ref) {
    VROSceneController *scene_controller = reinterpret_cast<VROSceneController *>(native_scene_controller_ref);
    std::shared_ptr<VROSceneController> shared_controller = std::shared_ptr<VROSceneController>(scene_controller);
    Renderer::native(native_renderer)->setSceneController(shared_controller);
}

}  // extern "C"
