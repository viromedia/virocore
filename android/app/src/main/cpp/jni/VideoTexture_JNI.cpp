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

JNI_METHOD(void, nativeCreateVideoTexture)(JNIEnv *env,
                                                jobject object) {
    jobject temporaryGlobalObj = env->NewGlobalRef(object);

    VROPlatformDispatchAsyncRenderer([temporaryGlobalObj] {
        std::shared_ptr<VROVideoTextureAVP> videoTexture = std::make_shared<VROVideoTextureAVP>();

        // Attach delegate callbacks to be triggered across the JNI bridge
        JNIEnv *env = VROPlatformGetJNIEnv();
        std::shared_ptr<VideoDelegate> delegateRef = std::make_shared<VideoDelegate>(temporaryGlobalObj);
        videoTexture->setDelegate(delegateRef);
        env->DeleteGlobalRef(temporaryGlobalObj);

        // Notify delegates that the video texture is ready to be used.
        delegateRef->onReady(VideoTexture::jptr(videoTexture));
    });
}

JNI_METHOD(void, nativeDeleteVideoTexture)(JNIEnv *env,
                                            jclass clazz,
                                            jlong textureRef) {
    VROPlatformDispatchAsyncRenderer([textureRef] {
        delete reinterpret_cast<PersistentRef<VROVideoTextureAVP> *>(textureRef);
    });
}

JNI_METHOD(void, nativePause)(JNIEnv *env,
                                        jclass clazz,
                                        jlong textureRef) {
    VROPlatformDispatchAsyncRenderer([textureRef] {
        VideoTexture::native(textureRef)->pause();
    });
}

JNI_METHOD(void, nativePlay)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef) {
    VROPlatformDispatchAsyncRenderer([textureRef] {
        VideoTexture::native(textureRef)->play();
    });
}

JNI_METHOD(void, nativeSetMuted)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef,
                                     jboolean muted) {
    VROPlatformDispatchAsyncRenderer([textureRef, muted] {
        VideoTexture::native(textureRef)->setMuted(muted);
    });
}

JNI_METHOD(void, nativeSetVolume)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef,
                                     jfloat volume) {
    VROPlatformDispatchAsyncRenderer([textureRef, volume] {
        VideoTexture::native(textureRef)->setVolume(volume);
    });
}

JNI_METHOD(void, nativeSetLoop)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef,
                                     jboolean loop) {
    VROPlatformDispatchAsyncRenderer([textureRef, loop] {
        VideoTexture::native(textureRef)->setLoop(loop);
    });
}

JNI_METHOD(void, nativeSeekToTime)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef,
                                     jint seconds) {
    VROPlatformDispatchAsyncRenderer([textureRef, seconds] {
        VideoTexture::native(textureRef)->seekToTime(seconds);
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

    VROPlatformDispatchAsyncRenderer([textureRef, renderContextRef, strVideoSource] {
        std::shared_ptr<RenderContext> renderContext = RenderContext::native(renderContextRef);
        std::shared_ptr<VROFrameSynchronizer> frameSynchronizer = renderContext->getFrameSynchronizer();
        std::shared_ptr<VRODriver> driver = renderContext->getDriver();
        std::shared_ptr<VROVideoTextureAVP> texture = VideoTexture::native(textureRef);
        texture->loadVideo(strVideoSource, frameSynchronizer, *driver.get());
        texture->prewarm();
    });

    env->ReleaseStringUTFChars(source, cVideoSource);
}

}  // extern "C"