//
//  VRORenderer_JNI.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <android/log.h>
#include <jni.h>

#include <memory>

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "VROSceneRendererCardboard.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_ViroActivity_##method_name

namespace {

inline jlong jptr(TreasureHuntRenderer *native_treasure_hunt) {
  return reinterpret_cast<intptr_t>(native_treasure_hunt);
}

inline TreasureHuntRenderer *native(jlong ptr) {
  return reinterpret_cast<TreasureHuntRenderer *>(ptr);
}
}  // anonymous namespace

extern "C" {

JNI_METHOD(jlong, nativeCreateRenderer)(JNIEnv *env, jclass clazz,
                                        jobject class_loader,
                                        jobject android_context,
                                        jlong native_gvr_api) {
  std::unique_ptr<gvr::AudioApi> audio_context(new gvr::AudioApi);
  audio_context->Init(env, android_context, class_loader,
                      GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);

  return jptr(
      new TreasureHuntRenderer(reinterpret_cast<gvr_context *>(native_gvr_api),
                               std::move(audio_context)));
}

JNI_METHOD(void, nativeDestroyRenderer)
(JNIEnv *env, jclass clazz, jlong native_treasure_hunt) {
  delete native(native_treasure_hunt);
}

JNI_METHOD(void, nativeInitializeGl)(JNIEnv *env, jobject obj,
                                     jlong native_treasure_hunt) {
  native(native_treasure_hunt)->InitializeGl();
}

JNI_METHOD(void, nativeDrawFrame)(JNIEnv *env, jobject obj,
                                  jlong native_treasure_hunt) {
  native(native_treasure_hunt)->DrawFrame();
}

JNI_METHOD(void, nativeOnTriggerEvent)(JNIEnv *env, jobject obj,
                                       jlong native_treasure_hunt) {
  native(native_treasure_hunt)->OnTriggerEvent();
}

JNI_METHOD(void, nativeOnPause)(JNIEnv *env, jobject obj,
                                jlong native_treasure_hunt) {
  native(native_treasure_hunt)->OnPause();
}

JNI_METHOD(void, nativeOnResume)(JNIEnv *env, jobject obj,
                                 jlong native_treasure_hunt) {
  native(native_treasure_hunt)->OnResume();
}

}  // extern "C"
