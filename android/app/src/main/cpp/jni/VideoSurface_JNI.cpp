//
//  VideoSurface_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include <VROVideoSurface.h>
#include <VROVideoTextureAVP.h>
#include "VRONode.h"

#include "VROMaterial.h"
#include "PersistentRef.h"
#include "VROFrameSynchronizer.h"
#include "VideoSurface_JNI.h"
#include "VRORenderer_JNI.h"
#include "Node_JNI.h"
#include "RenderContext_JNI.h"
#include "VROLog.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_VideoSurfaceJni_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateVideo)(JNIEnv *env,
                                        jobject object,
                                        jfloat width,
                                        jfloat height,
                                        jstring url,
                                        jlong renderContextRef) {
    // Grab required objects from the RenderContext required for initialization
    std::shared_ptr<RenderContext> renderContext = RenderContext::native(renderContextRef);
    std::shared_ptr<VROFrameSynchronizer> frameSynchronizer = renderContext->getFrameSynchronizer();
    std::shared_ptr<VRODriver> driver = renderContext->getDriver();

    // Create and set the videoTexture on the video surface geometry
    const char *videoSource = env->GetStringUTFChars(url, JNI_FALSE);
    std::shared_ptr<VROVideoTextureAVP> videoTexture = std::make_shared<VROVideoTextureAVP>();
    std::shared_ptr<VROVideoSurface> surface = VROVideoSurface::createVideoSurface(width,
                                                                                   height,
                                                                                   videoSource,
                                                                                   frameSynchronizer,
                                                                                   videoTexture,
                                                                                   *driver.get());
    env->ReleaseStringUTFChars(url, videoSource);

    // Attach delegate callbacks to be triggered across the JNI bridge
    std::shared_ptr<VideoDelegate> delegateRef = std::make_shared<VideoDelegate>(object, env);
    surface->setDelegate(delegateRef);

    // Return a jlong point reference to the created VideoSurface object
    return VideoSurface::jptr(surface);
}

JNI_METHOD(void, nativeDeleteVideo)(JNIEnv *env,
                              jclass clazz,
                              jlong native_video_ref) {
    delete reinterpret_cast<PersistentRef<VROVideoSurface> *>(native_video_ref);
}

JNI_METHOD(void, nativePause)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_video_ref) {
    VideoSurface::native(native_video_ref)->pause();
}

JNI_METHOD(void, nativePlay)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_video_ref) {
    VideoSurface::native(native_video_ref)->play();
}

JNI_METHOD(jboolean , nativeIsPaused)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_video_ref) {
    return VideoSurface::native(native_video_ref)->isPaused();
}

JNI_METHOD(void, nativeSetMuted)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_video_ref,
                                     jboolean muted) {
    VideoSurface::native(native_video_ref)->setMuted(muted);
}

JNI_METHOD(void, nativeSetVolume)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_video_ref,
                                     jfloat volume) {
    VideoSurface::native(native_video_ref)->setVolume(volume);
}

JNI_METHOD(void, nativeSetLoop)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_video_ref,
                                     jboolean loop) {
    VideoSurface::native(native_video_ref)->setLoop(loop);
}

JNI_METHOD(void, nativeSeekToTime)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_video_ref,
                                     jint seconds) {
    VideoSurface::native(native_video_ref)->seekToTime(seconds);
}


JNI_METHOD(void, nativeAttachToNode)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_video_ref,
                                     jlong native_node_ref) {
    std::shared_ptr<VROVideoSurface> videoSurface = VideoSurface::native(native_video_ref);
    Node::native(native_node_ref)->setGeometry(videoSurface);
}

}  // extern "C"

VideoDelegate::VideoDelegate(jobject javaVideoObject, JNIEnv *env){
    _javaObject = reinterpret_cast<jclass>(env->NewGlobalRef(javaVideoObject));
    _env = env;
}

VideoDelegate::~VideoDelegate() {
    _env->DeleteGlobalRef(_javaObject);
}

void VideoDelegate::videoDidFinish() {
    _env->ExceptionClear();
    jclass viroClass = _env->FindClass("com/viro/renderer/jni/VideoSurfaceJni");
    if (viroClass == nullptr) {
        perr("Unable to find VideoSurfaceJni class for nativeOnVideoFinished() callback.");
        return;
    }

    jmethodID method = _env->GetMethodID(viroClass, "nativeOnVideoFinished", "()V");
    if (method == nullptr) {
        perr("Unable to find method nativeOnVideoFinished() callback.");
        return;
    }

    _env->CallVoidMethod(_javaObject, method);
    if (_env->ExceptionOccurred()) {
        perr("Exception occured when calling nativeOnVideoFinished.");
        _env->ExceptionClear();
    }
    _env->DeleteLocalRef(viroClass);
}