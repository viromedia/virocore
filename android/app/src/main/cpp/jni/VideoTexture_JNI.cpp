//
//  VideoTexture_JNI.cpp
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
#include "VRORenderer_JNI.h"
#include "RenderContext_JNI.h"
#include "VideoTexture_JNI.h"
#include "VideoDelegate_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_VideoTextureJni_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateVideoTexture)(JNIEnv *env,
                                           jobject object) {

    std::shared_ptr<VROVideoTextureAVP> videoTexture = std::make_shared<VROVideoTextureAVP>();
    VROPlatformDispatchAsyncRenderer([videoTexture] {
        videoTexture->bindSurface();
    });

    return VideoTexture::jptr(videoTexture);
}

JNI_METHOD(jlong, nativeCreateVideoDelegate)(JNIEnv *env,
                                             jobject object) {

    std::shared_ptr<VideoDelegate> delegate = std::make_shared<VideoDelegate>(object);
    return VideoDelegate::jptr(delegate);
}

JNI_METHOD(void, nativeAttachDelegate)(JNIEnv *env,
                                      jobject object,
                                      jlong textureRef, jlong delegateRef) {

    std::shared_ptr<VROVideoTextureAVP> videoTexture = VideoTexture::native(textureRef);
    std::shared_ptr<VideoDelegate> videoDelegate = VideoDelegate::native(delegateRef);

    VROPlatformDispatchAsyncRenderer([videoTexture, videoDelegate] {
        videoTexture->setDelegate(videoDelegate);
        // Notify delegates that the video texture is ready to be used.
        videoDelegate->onReady();
    });
}

JNI_METHOD(void, nativeDeleteVideoTexture)(JNIEnv *env,
                                            jclass clazz,
                                            jlong textureRef) {
    delete reinterpret_cast<PersistentRef<VROVideoTextureAVP> *>(textureRef);
}

JNI_METHOD(void, nativeDeleteVideoDelegate)(JNIEnv *env,
                                             jobject object,
                                             jlong delegateRef) {
    delete reinterpret_cast<PersistentRef<VideoDelegate> *>(delegateRef);
}

JNI_METHOD(void, nativePause)(JNIEnv *env,
                                        jclass clazz,
                                        jlong textureRef) {
    std::shared_ptr<VROVideoTextureAVP> videoTexture = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture] {
        videoTexture->pause();
    });
}

JNI_METHOD(void, nativePlay)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef) {

    std::shared_ptr<VROVideoTextureAVP> videoTexture = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture] {
        videoTexture->play();
    });
}

JNI_METHOD(void, nativeSetMuted)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef,
                                     jboolean muted) {

    std::shared_ptr<VROVideoTextureAVP> videoTexture = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture, muted] {
        videoTexture->setMuted(muted);
    });
}

JNI_METHOD(void, nativeSetVolume)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef,
                                     jfloat volume) {

    std::shared_ptr<VROVideoTextureAVP> videoTexture = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture, volume] {
        videoTexture->setVolume(volume);
    });
}

JNI_METHOD(void, nativeSetLoop)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef,
                                     jboolean loop) {

    std::shared_ptr<VROVideoTextureAVP> videoTexture = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture, loop] {
        videoTexture->setLoop(loop);
    });
}

JNI_METHOD(void, nativeSeekToTime)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef,
                                     jint seconds) {

    std::shared_ptr<VROVideoTextureAVP> videoTexture = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture, seconds] {
        videoTexture->seekToTime(seconds);
    });
}

JNI_METHOD(void, nativeLoadSource)(JNIEnv *env,
                                   jclass clazz,
                                   jlong textureRef,
                                   jstring source,
                                   jlong renderContextRef) {
    // Grab required objects from the RenderContext required for initialization
    const char *cVideoSource = env->GetStringUTFChars(source, JNI_FALSE);
    std::string strVideoSource(cVideoSource);

    std::shared_ptr<VROVideoTextureAVP> videoTexture = VideoTexture::native(textureRef);
    std::shared_ptr<RenderContext> renderContext = RenderContext::native(renderContextRef);

    VROPlatformDispatchAsyncRenderer([videoTexture, renderContext, strVideoSource] {
        std::shared_ptr<VROFrameSynchronizer> frameSynchronizer = renderContext->getFrameSynchronizer();
        std::shared_ptr<VRODriver> driver = renderContext->getDriver();
        videoTexture->loadVideo(strVideoSource, frameSynchronizer, *driver.get());
        videoTexture->prewarm();
    });

    env->ReleaseStringUTFChars(source, cVideoSource);
}

}  // extern "C"