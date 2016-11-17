//
//  VRORenderer_JNI.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "VROSceneRendererCardboard.h"
#include "VROPlatformUtil.h"
#include "VROSample.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_ViroActivity_##method_name

namespace {

inline jlong jptr(VROSceneRendererCardboard *native_renderer) {
  return reinterpret_cast<intptr_t>(native_renderer);
}

inline VROSceneRendererCardboard *native(jlong ptr) {
  return reinterpret_cast<VROSceneRendererCardboard *>(ptr);
}
}  // anonymous namespace

extern "C" {

static std::shared_ptr<VROSample> sample;

JNI_METHOD(jlong, nativeCreateRenderer)(JNIEnv *env, jclass clazz,
                                        jobject class_loader,
                                        jobject activity,
                                        jobject android_context,
                                        jobject asset_mgr,
                                        jlong native_gvr_api) {
  std::shared_ptr<gvr::AudioApi> gvrAudio = std::make_shared<gvr::AudioApi>();
  gvrAudio->Init(env, android_context, class_loader, GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
  VROPlatformSetEnv(env, activity, asset_mgr);

  return jptr(
      new VROSceneRendererCardboard(reinterpret_cast<gvr_context *>(native_gvr_api), gvrAudio));
}

JNI_METHOD(void, nativeDestroyRenderer)
(JNIEnv *env, jclass clazz, jlong native_renderer) {
  VROPlatformReleaseEnv();
  delete native(native_renderer);
}

JNI_METHOD(void, nativeInitializeGl)(JNIEnv *env, jobject obj,
                                     jlong native_renderer) {

  // TODO Temporary place for sample
  sample = std::make_shared<VROSample>();

  VROSceneRendererCardboard *sceneRenderer = native(native_renderer);
  sceneRenderer->setRenderDelegate(sample);
  sceneRenderer->setSceneController(sample->loadBoxScene(sceneRenderer->getFrameSynchronizer(),
                                                         sceneRenderer->getDriver()));
  sceneRenderer->initGL();
}

JNI_METHOD(void, nativeDrawFrame)(JNIEnv *env, jobject obj,
                                  jlong native_renderer) {
  native(native_renderer)->onDrawFrame();
}

JNI_METHOD(void, nativeOnTriggerEvent)(JNIEnv *env, jobject obj,
                                       jlong native_renderer) {
  native(native_renderer)->onTriggerEvent();
}

JNI_METHOD(void, nativeOnPause)(JNIEnv *env, jobject obj,
                                jlong native_renderer) {
  native(native_renderer)->onPause();
}

JNI_METHOD(void, nativeOnResume)(JNIEnv *env, jobject obj,
                                 jlong native_renderer) {
  native(native_renderer)->onResume();
}

}  // extern "C"
