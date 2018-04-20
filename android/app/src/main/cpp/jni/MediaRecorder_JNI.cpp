/*
 * Copyright © 2017 Viro Media. All rights reserved.
 */
#include <memory>
#include <VROPlatformUtil.h>
#include <VRORenderToTextureDelegateAndroid.h>
#include "MediaRecorder_JNI.h"
#include "VRORenderer_JNI.h"
#include "VROChoreographer.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ViroMediaRecorder_##method_name
#endif

extern "C" {
VRO_METHOD(jlong, nativeCreateNativeRecorder)(VRO_ARGS
                                              jlong rendererRef) {
    std::shared_ptr<MediaRecorder_JNI> recorder = std::make_shared<MediaRecorder_JNI>(obj, env);
    std::weak_ptr<VROSceneRenderer> renderer_w = Renderer::native(rendererRef);

    VROPlatformDispatchAsyncRenderer([renderer_w, recorder] {
        std::shared_ptr<VROSceneRenderer> renderer = renderer_w.lock();
        if (renderer) {
            recorder->nativeCreateRecorder(renderer);
        }
    });
    return MediaRecorder::jptr(recorder);
}

VRO_METHOD(void, nativeDeleteNativeRecorder)(VRO_ARGS
                                            jlong jRecorderRef) {
    delete reinterpret_cast<PersistentRef<MediaRecorder_JNI> *>(jRecorderRef);

}

VRO_METHOD(void, nativeEnableFrameRecording)(VRO_ARGS
                                             jlong jRecorderRef,
                                             jboolean jIsRecording) {
    std::shared_ptr<MediaRecorder_JNI> recorder = MediaRecorder::native(jRecorderRef);
    VROPlatformDispatchAsyncRenderer([recorder, jIsRecording] {
        recorder->nativeEnableFrameRecording(jIsRecording);
    });
}

VRO_METHOD(void, nativeScheduleScreenCapture)(VRO_ARGS
                                              jlong jRecorderRef) {
    std::shared_ptr<MediaRecorder_JNI> recorder = MediaRecorder::native(jRecorderRef);
    VROPlatformDispatchAsyncRenderer([recorder] {
        recorder->nativeScheduleScreenCapture();
    });
}
} // extern "C"


MediaRecorder_JNI::MediaRecorder_JNI(jobject recorderJavaObject, JNIEnv *env) {
    _javaMediaRecorder = reinterpret_cast<jclass>(VROPlatformGetJNIEnv()->NewWeakGlobalRef(recorderJavaObject));
}

MediaRecorder_JNI::~MediaRecorder_JNI() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->DeleteWeakGlobalRef(_javaMediaRecorder);
}

/*
 * Calls from Java to Native
 */
void MediaRecorder_JNI::nativeCreateRecorder(std::shared_ptr<VROSceneRenderer> renderer) {
    // Create the VROAndroidRecorder representing this Media Jni Recorder through which all calls are routed to.
    _nativeMediaRecorder = std::make_shared<VROAVRecorderAndroid>(shared_from_this());
    _nativeMediaRecorder->init(renderer->getDriver());

    // Attach the recorder's renderToTextureDelegate for inputing recording data to our choreographer.
    std::shared_ptr<VRORenderToTextureDelegateAndroid> delegate = _nativeMediaRecorder->getRenderToTextureDelegate();
    renderer->getRenderer()->getChoreographer()->setRenderToTextureDelegate(delegate);
    renderer->getRenderer()->getChoreographer()->setRenderToTextureEnabled(true);
}

void MediaRecorder_JNI::nativeEnableFrameRecording(bool isRecording) {
    _nativeMediaRecorder->setEnableVideoFrameRecording(isRecording);
}

void MediaRecorder_JNI::nativeScheduleScreenCapture() {
    _nativeMediaRecorder->scheduleScreenCapture();
}

/*
 * Native to Java calls.
 */
void MediaRecorder_JNI::onBindToEGLSurface() {
    VROPlatformCallJavaFunction(_javaMediaRecorder, "onNativeBindToEGLSurface","()V");
}

void MediaRecorder_JNI::onUnBindFromEGLSurface() {
    VROPlatformCallJavaFunction(_javaMediaRecorder, "onNativeUnBindEGLSurface","()V");
}

void MediaRecorder_JNI::onEnableFrameRecording(bool enabled) {
    VROPlatformCallJavaFunction(_javaMediaRecorder, "onNativeEnableFrameRecording","(Z)V", enabled);
}

void MediaRecorder_JNI::onEglSwap() {
    VROPlatformCallJavaFunction(_javaMediaRecorder, "onNativeSwapEGLSurface","()V");
}

void MediaRecorder_JNI::onTakeScreenshot() {
    VROPlatformCallJavaFunction(_javaMediaRecorder, "onNativeTakeScreenshot","()V");
}