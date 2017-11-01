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
#include <VROTextureUtil.h>
#include "VRONode.h"
#include "VROMaterial.h"
#include "PersistentRef.h"
#include "VROFrameSynchronizer.h"
#include "VRORenderer_JNI.h"
#include "ViroContext_JNI.h"
#include "VideoTexture_JNI.h"
#include "VideoDelegate_JNI.h"
#include "VRODriverOpenGL.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_VideoTexture_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateVideoTexture)(JNIEnv *env,
                                            jobject object,
                                            jlong context_j,
                                            jstring stereoMode) {
    VROStereoMode mode = VROTextureUtil::getStereoModeForString(VROPlatformGetString(stereoMode));
    std::weak_ptr<ViroContext> context_w = ViroContext::native(context_j);
    std::shared_ptr<VROVideoTextureAVP> videoTexture = std::make_shared<VROVideoTextureAVP>(mode);
    videoTexture->init();

    VROPlatformDispatchAsyncRenderer([videoTexture, context_w] {
        std::shared_ptr<ViroContext> context = context_w.lock();
        if (!context) {
            return;
        }

        videoTexture->bindSurface(std::dynamic_pointer_cast<VRODriverOpenGL>(context->getDriver()));
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

    std::weak_ptr<VROVideoTextureAVP> videoTexture_w = VideoTexture::native(textureRef);
    std::weak_ptr<VideoDelegate> videoDelegate_w = VideoDelegate::native(delegateRef);

    VROPlatformDispatchAsyncRenderer([videoTexture_w, videoDelegate_w] {
        std::shared_ptr<VROVideoTextureAVP> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        std::shared_ptr<VideoDelegate> videoDelegate = videoDelegate_w.lock();
        if (!videoDelegate) {
            return;
        }

        videoTexture->setDelegate(videoDelegate);
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
    std::weak_ptr<VROVideoTextureAVP> videoTexture_w = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture_w] {
        std::shared_ptr<VROVideoTextureAVP> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        videoTexture->pause();
    });
}

JNI_METHOD(void, nativePlay)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef) {

    std::weak_ptr<VROVideoTextureAVP> videoTexture_w = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture_w] {
        std::shared_ptr<VROVideoTextureAVP> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        videoTexture->play();
    });
}

JNI_METHOD(void, nativeSetMuted)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef,
                                     jboolean muted) {

    std::weak_ptr<VROVideoTextureAVP> videoTexture_w = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture_w, muted] {
        std::shared_ptr<VROVideoTextureAVP> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        videoTexture->setMuted(muted);
    });
}

JNI_METHOD(void, nativeSetVolume)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef,
                                     jfloat volume) {

    std::weak_ptr<VROVideoTextureAVP> videoTexture_w = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture_w, volume] {
        std::shared_ptr<VROVideoTextureAVP> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        videoTexture->setVolume(volume);
    });
}

JNI_METHOD(void, nativeSetLoop)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef,
                                     jboolean loop) {

    std::weak_ptr<VROVideoTextureAVP> videoTexture_w = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture_w, loop] {
        std::shared_ptr<VROVideoTextureAVP> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        videoTexture->setLoop(loop);
    });
}

JNI_METHOD(void, nativeSeekToTime)(JNIEnv *env,
                                     jclass clazz,
                                     jlong textureRef,
                                     jfloat seconds) {

    std::weak_ptr<VROVideoTextureAVP> videoTexture_w = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture_w, seconds] {
        std::shared_ptr<VROVideoTextureAVP> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        videoTexture->seekToTime(seconds);
    });
}

JNI_METHOD(void, nativeLoadSource)(JNIEnv *env,
                                   jclass clazz,
                                   jlong textureRef,
                                   jstring source,
                                   jlong context_j) {
    // Grab required objects from the RenderContext required for initialization
    std::string strVideoSource = VROPlatformGetString(source);
    std::weak_ptr<VROVideoTextureAVP> videoTexture_w = VideoTexture::native(textureRef);
    std::weak_ptr<ViroContext> context_w = ViroContext::native(context_j);

    VROPlatformDispatchAsyncRenderer([videoTexture_w, context_w, strVideoSource] {
        std::shared_ptr<VROVideoTextureAVP> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        std::shared_ptr<ViroContext> context = context_w.lock();
        if (!context) {
            return;
        }

        std::shared_ptr<VROFrameSynchronizer> frameSynchronizer = context->getFrameSynchronizer();
        std::shared_ptr<VRODriver> driver = context->getDriver();
        videoTexture->loadVideo(strVideoSource, frameSynchronizer, driver);
        videoTexture->prewarm();
    });
}

}  // extern "C"